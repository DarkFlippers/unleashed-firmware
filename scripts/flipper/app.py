import logging
import argparse
import sys
import colorlog


class App:
    def __init__(self, no_exit=False):
        # Argument Parser
        self.no_exit = no_exit
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        # Logging
        self.logger = colorlog.getLogger()
        # Application specific initialization
        self.init()

    def __call__(self, args=None):
        self.args, self.other_args = self.parser.parse_known_args(args=args)
        # configure log output
        self.log_level = logging.DEBUG if self.args.debug else logging.INFO
        self.logger.setLevel(self.log_level)
        if not self.logger.hasHandlers():
            self.handler = colorlog.StreamHandler(sys.stdout)
            self.handler.setLevel(self.log_level)
            self.formatter = colorlog.ColoredFormatter(
                "%(log_color)s%(asctime)s [%(levelname)s] %(message)s",
                log_colors={
                    "DEBUG": "cyan",
                    # "INFO": "white",
                    "WARNING": "yellow",
                    "ERROR": "red",
                    "CRITICAL": "red,bg_white",
                },
            )
            self.handler.setFormatter(self.formatter)
            self.logger.addHandler(self.handler)

        # execute requested function
        self.before()
        return_code = self.call()
        self.after()
        if isinstance(return_code, int):
            return self._exit(return_code)
        else:
            self.logger.error("Missing return code")
            return self._exit(255)

    def _exit(self, code):
        if self.no_exit:
            return code
        exit(code)

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
