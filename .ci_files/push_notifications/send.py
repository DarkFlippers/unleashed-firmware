import argparse
import json
import os
from pathlib import Path

import firebase_admin
from firebase_admin import credentials, messaging

DEFAULT_KEY_PATH = Path("key.json")
SERVICE_ACCOUNT_ENV = "FIREBASE_SERVICE_ACCOUNT_JSON"

BUILD_TARGETS = {
    "release": {
        "topic": "unl_release",
        "title": "New Unleashed firmware released",
    },
    "dev": {
        "topic": "unl_dev",
        "title": "New Unleashed FW Dev Build",
    },
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("build_type", choices=BUILD_TARGETS.keys())
    parser.add_argument("version")
    parser.add_argument("--commit-sha", default="")
    return parser.parse_args()


def load_service_account() -> dict | str:
    raw_service_account = os.getenv(SERVICE_ACCOUNT_ENV)
    if raw_service_account:
        try:
            return json.loads(raw_service_account)
        except json.JSONDecodeError:
            service_account_path = Path(raw_service_account)
            if service_account_path.exists():
                return str(service_account_path)
            raise

    if DEFAULT_KEY_PATH.exists():
        return str(DEFAULT_KEY_PATH)

    raise FileNotFoundError(
        f"Set {SERVICE_ACCOUNT_ENV} or provide {DEFAULT_KEY_PATH} next to the script"
    )


def build_message(build_type: str, version: str, commit_sha: str) -> tuple[str, str, str]:
    target = BUILD_TARGETS[build_type]

    if build_type == "release":
        body = f"Version: {version}"
    else:
        body = f"Build: {version}"
        if commit_sha:
            body = f"{body} | Commit: {commit_sha[:7]}"

    return target["topic"], target["title"], body


def main() -> None:
    args = parse_args()
    topic, title, body = build_message(args.build_type, args.version, args.commit_sha)

    firebase_admin.initialize_app(credentials.Certificate(load_service_account()))

    message = messaging.Message(
        notification=messaging.Notification(title=title, body=body),
        topic=topic,
    )

    message_id = messaging.send(message)
    print("sent:", message_id)


if __name__ == "__main__":
    main()
