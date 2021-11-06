import os
import sys
import glob
import hashlib

import json

import jinja2
import modm_devices

from pathlib import Path

Import('env')

class DevicesCache(dict):
    """
    Building the device enumeration from modm-device is quite expensive,
    so we cache the results in `ext/modm-devices.cache`
    The text file contains two maps:
      1. partname -> file-name.xml
         We use this to populate the `:target` option, but we only
         actually parse the device file and build the device on the first
         access of the value.
      2. file-name.xml -> MD5 hash
         This is used to check if any files have changed their contents.
         No additional checks are done, if files have moved, this may fail.
    """

    def __init__(self):
        dict.__init__(self)
        self.device_to_file = {}

    def parse_all(self):
        mapping = {}
        device_file_names = glob.glob(modm_devices.pkg.get_filename('modm_devices', "resources/devices/**/*.xml"))

        # roughly filter to supported devices
        supported = ["stm32f0", "stm32f1", "stm32f2", "stm32f3", "stm32f4", "stm32f7",
                     "stm32g0", "stm32g4",
                     "stm32l1", "stm32l4",
                     "at90", "attiny", "atmega",
                     "samd21",
                     "hosted"]
        device_file_names = [dfn for dfn in device_file_names if any(s in dfn for s in supported)]

        # Parse the files and build the :target enumeration
        parser = modm_devices.parser.DeviceParser()
        for device_file_name in device_file_names:
            device_file = parser.parse(device_file_name)
            for device in device_file.get_devices():
                self[device.partname] = device
                mapping[device.partname] = device_file_name

        return mapping

    def build(self):
        cache = Path(".pio/modm-devices.cache")
        repos = {p:None for p in [".", modm_devices.pkg.get_filename('modm_devices', "resources/devices")]}
        recompute_required = not cache.exists()

        if cache.exists():
            # Read cache file and populate :target
            for line in cache.read_text().splitlines():
                line = line.split(" ")
                if line[0].startswith("/"):
                    file = line[0][1:]

                    # Store repo shas
                    if file in repos.keys():
                        repos[file] = line[1]
                        continue

                    # Check normal files
                    file = Path(file)
                    if not file.exists() or (line[1] != hashlib.md5(file.read_bytes()).hexdigest()):
                        recompute_required = True
                        break
                else:
                    # Store None as device file value
                    self.device_to_file[line[0]] = line[1]
                    self[line[0]] = None

        if recompute_required:
            print(">> modm: Recomputing device cache...")
            content = self.parse_all()
            files = ["{} {}".format(Path(f), hashlib.md5(Path(f).read_bytes()).hexdigest()) for f in set(content.values())]
            content = ["{} {}".format(d, Path(f)) for (d, f) in content.items()]
            repos = ["{} {}".format(path, sha) for path, sha in repos.items()]
            content = sorted(content) + sorted(files + repos)
            cache.write_text("\n".join(content))

    def __getitem__(self, item):
        value = dict.__getitem__(self, item)
        if value is None:
            # Parse the device file and build its devices
            parser = modm_devices.parser.DeviceParser()
            device_file = parser.parse(self.device_to_file[item])
            for device in device_file.get_devices():
                self[device.partname] = device
            return self[item]
        return value

class Target:
    def __init__(self):
        devices = DevicesCache()
        devices.build()
        self.name = env.GetProjectOption("target")
        with open(os.path.join("targets", self.name + ".json")) as f:
            self.target = json.load(f)
        self.board = env.BoardConfig()
        self.dev = devices[self.board.get("device")]

    def get_pin(self, name):
        port = name[1].lower()
        pin = name[2:].lower()
        for g in self.dev.get_driver("gpio")["gpio"]:
            if g['port'] == port and g['pin'] == pin:
                return g
        return None

    def has_port(self, name, instance):
        driver = self.dev.get_driver(name)
        for d in driver["instance"]:
            if d == str(instance):
                return True
        return False

    def get_signal(self, pin, signal, instance):
        p = self.get_pin(pin)
        if p is None:
            return None
        for s in p["signal"]:
            if s["driver"].lower() == signal.lower() and s["instance"] == str(instance):
                return s
        return None

    def get_af(self, pin, signal, instance):
        if pin is None:
            return None;
        return {
            "pin": pin,
            "af": self.get_signal(pin, signal, instance)["af"],
        }

    def build(self):
        info = {
            "name": self.target["name"],
            "system": self.board.get("system"),
            "ports": {},
            "devices": self.target["devices"],
        }

        if self.target["ports"]["uart"]:
            info["ports"]["uart"] = []
            for u in self.target["ports"]["uart"]:
                port_type = "usart"
                if not self.has_port("usart", u["index"]):
                    if not self.has_port("uart", u["index"]):
                        raise Exception("invalid uart" + u["index"])
                    else:
                        port_type = "uart"
                port = {
                    "index": u["index"],
                    "type": port_type.upper(),
                    "rx": self.get_af(u["rx"], port_type, u["index"]),
                    "tx": self.get_af(u["tx"], port_type, u["index"])
                }
                info["ports"]["uart"].append(port)

        if self.target["ports"]["spi"]:
            info["ports"]["spi"] = []
            for s in self.target["ports"]["spi"]:
                port = {
                    "index": s["index"],

                    "sck": self.get_af(s["sck"], "spi", s["index"]),
                    "miso": self.get_af(s["miso"], "spi", s["index"]),
                    "mosi": self.get_af(s["mosi"], "spi", s["index"]),

                    "dma": s["dma"],
                }
                info["ports"]["spi"].append(port)

        print(json.dumps(info, sort_keys=False, indent=4))
        return info


target = Target()

with open('include/target/target.h.j2') as template:
  template = jinja2.Template(template.read())
  with open('include/target/target.h', 'w') as header:
    header.write(template.render(target.build()))