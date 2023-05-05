#!/usr/bin/env python3

import argparse
import os
import re
import sys

from slack_sdk import WebClient
from slack_sdk.errors import SlackApiError


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("slack_token")
    parser.add_argument("slack_channel")
    args = parser.parse_args()
    return args


def checkCommitMessage(msg):
    regex = re.compile(r"^'?\[(FL-\d+,?\s?)+\]")
    if regex.match(msg):
        return True
    return False


def reportSlack(commit_hash, slack_token, slack_channel, message):
    client = WebClient(token=slack_token)
    try:
        client.chat_postMessage(channel="#" + slack_channel, text=message)
    except SlackApiError as e:
        print(e)
        sys.exit(1)


def main():
    args = parse_args()
    commit_msg = os.getenv("COMMIT_MSG")
    commit_hash = os.getenv("COMMIT_HASH")
    commit_sha = os.getenv("COMMIT_SHA")
    commit_link = (
        "<https://github.com/flipperdevices/flipperzero-firmware/commit/"
        + commit_hash
        + "|"
        + commit_sha
        + ">"
    )
    message = "Commit " + commit_link + " merged to dev without 'FL' ticket!"
    if not checkCommitMessage(commit_msg):
        reportSlack(commit_hash, args.slack_token, args.slack_channel, message)


if __name__ == "__main__":
    main()
