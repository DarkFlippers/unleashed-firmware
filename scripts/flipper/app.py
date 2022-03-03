import logging
import argparse
import sys
import os


class App:
    def __init__(self):
        # Argument Parser
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        # Logging
        self.logger = logging.getLogger()
        # Application specific initialization
        self.init()

    def __call__(self):
        self.args, _ = self.parser.parse_known_args()
        # configure log output
        self.log_level = logging.DEBUG if self.args.debug else logging.INFO
        self.logger.setLevel(self.log_level)
        self.handler = logging.StreamHandler(sys.stdout)
        self.handler.setLevel(self.log_level)
        self.formatter = logging.Formatter("%(asctime)s [%(levelname)s] %(message)s")
        self.handler.setFormatter(self.formatter)
        self.logger.addHandler(self.handler)

        # execute requested function
        self.before()
        return_code = self.call()
        self.after()
        if isinstance(return_code, int):
            exit(return_code)
        else:
            self.logger.error(f"Missing return code")
            exit(255)

    def call(self):
        if "func" not in self.args:
            self.parser.error("Choose something to do")
        return self.args.func()

    def init(self):
        raise Exception("init() is not implemented")

    def before(self):
        pass

    def after(self):
        pass
