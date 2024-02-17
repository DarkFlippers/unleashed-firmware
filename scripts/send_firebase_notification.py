import argparse
import logging
from firebase_admin import messaging, credentials, initialize_app


class FirebaseNotifications:
    def __init__(self, service_account_file):
        try:
            cred = credentials.Certificate(service_account_file)
            self.firebase_app = initialize_app(cred)
        except Exception as e:
            logging.exception(e)
            raise e

    def send(self, title, body, condition):
        try:
            message = messaging.Message(
                notification=messaging.Notification(title=title, body=body),
                condition=condition,
            )
            messaging.send(message, app=self.firebase_app)
        except Exception as e:
            logging.exception(e)
            raise e


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--token_file", help="Firebase token file", required=True)
    parser.add_argument(
        "--version", help="Firmware version to notify with", required=True
    )
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    args = parse_args()
    notification = FirebaseNotifications(args.token_file)
    notification.send(
        title="Firmware Update Available",
        body=f"New firmware version is ready to install: {args.version}",
        condition="'flipper_update_firmware_release' in topics",
    )
