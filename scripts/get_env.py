#!/usr/bin/env python3

import ssl
import json
import os
import shlex
import re
import argparse
import datetime
import urllib.request

# event_file = open('${{ github.event_path }}')


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--event_file", help="Current GitHub event file", required=True)
    parser.add_argument(
        "--is_pull", help="Is it Pull Request", default=False, action="store_true"
    )
    args = parser.parse_args()
    return args


def get_commit_json(event):
    context = ssl._create_unverified_context()
    with urllib.request.urlopen(
        event["pull_request"]["_links"]["commits"]["href"], context=context
    ) as commit_file:
        commit_json = json.loads(commit_file.read().decode("utf-8"))
    return commit_json


def get_details(event, args):
    data = {}
    current_time = datetime.datetime.utcnow().date()
    if args.is_pull:
        commit_json = get_commit_json(event)
        data["commit_comment"] = shlex.quote(commit_json[-1]["commit"]["message"])
        data["commit_hash"] = commit_json[-1]["sha"]
        ref = event["pull_request"]["head"]["ref"]
        data["pull_id"] = event["pull_request"]["number"]
        data["pull_name"] = shlex.quote(event["pull_request"]["title"])
    else:
        data["commit_comment"] = shlex.quote(event["commits"][-1]["message"])
        data["commit_hash"] = event["commits"][-1]["id"]
        ref = event["ref"]
    data["commit_sha"] = data["commit_hash"][:8]
    data["branch_name"] = re.sub("refs/\w+/", "", ref)
    data["suffix"] = (
        data["branch_name"].replace("/", "_")
        + "-"
        + current_time.strftime("%d%m%Y")
        + "-"
        + data["commit_sha"]
    )
    if ref.startswith("refs/tags/"):
        data["suffix"] = data["branch_name"].replace("/", "_")
    return data


def add_envs(data, env_file, args):
    print(f'COMMIT_MSG={data["commit_comment"]}', file=env_file)
    print(f'COMMIT_HASH={data["commit_hash"]}', file=env_file)
    print(f'COMMIT_SHA={data["commit_sha"]}', file=env_file)
    print(f'SUFFIX={data["suffix"]}', file=env_file)
    print(f'BRANCH_NAME={data["branch_name"]}', file=env_file)
    print(f'DIST_SUFFIX={data["suffix"]}', file=env_file)
    print(f'WORKFLOW_BRANCH_OR_TAG={data["branch_name"]}', file=env_file)
    if args.is_pull:
        print(f'PULL_ID={data["pull_id"]}', file=env_file)
        print(f'PULL_NAME={data["pull_name"]}', file=env_file)


def main():
    args = parse_args()
    event_file = open(args.event_file)
    event = json.load(event_file)
    env_file = open(os.environ["GITHUB_ENV"], "a")
    data = get_details(event, args)
    add_envs(data, env_file, args)
    event_file.close()
    env_file.close()


if __name__ == "__main__":
    main()
