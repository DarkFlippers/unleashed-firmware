#!/usr/bin/env python3

import argparse
import datetime
import json
import os
import random
import re
import shlex
import ssl
import string
import urllib.request


def id_gen(size=5, chars=string.ascii_uppercase + string.digits):
    return "".join(random.choice(chars) for _ in range(size))


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--event_file", help="Current GitHub event file", required=True)
    parser.add_argument(
        "--type",
        help="Event file type",
        required=True,
        choices=["pull", "tag", "other"],
    )
    args = parser.parse_args()
    return args


def get_commit_json(event):
    context = ssl._create_unverified_context()
    commit_url = event["pull_request"]["base"]["repo"]["commits_url"].replace(
        "{/sha}", f"/{event['pull_request']['head']['sha']}"
    )
    with urllib.request.urlopen(commit_url, context=context) as commit_file:
        commit_json = json.loads(commit_file.read().decode("utf-8"))
    return commit_json


def get_details(event, args):
    data = {}
    current_time = datetime.datetime.utcnow().date()
    if args.type == "pull":
        commit_json = get_commit_json(event)
        data["commit_comment"] = shlex.quote(commit_json["commit"]["message"])
        data["commit_hash"] = commit_json["sha"]
        ref = event["pull_request"]["head"]["ref"]
        data["pull_id"] = event["pull_request"]["number"]
        data["pull_name"] = shlex.quote(event["pull_request"]["title"])
    elif args.type == "tag":
        data["commit_comment"] = shlex.quote(event["head_commit"]["message"])
        data["commit_hash"] = event["head_commit"]["id"]
        ref = event["ref"]
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


def add_env(name, value, file):
    delimeter = id_gen()
    print(f"{name}<<{delimeter}", file=file)
    print(f"{value}", file=file)
    print(f"{delimeter}", file=file)


def add_set_output_var(name, value, file):
    print(f"{name}={value}", file=file)


def add_envs(data, gh_env_file, gh_out_file, args):
    add_env("COMMIT_MSG", data["commit_comment"], gh_env_file)
    add_env("COMMIT_HASH", data["commit_hash"], gh_env_file)
    add_env("COMMIT_SHA", data["commit_sha"], gh_env_file)
    add_env("SUFFIX", data["suffix"], gh_env_file)
    add_env("BRANCH_NAME", data["branch_name"], gh_env_file)
    add_env("DIST_SUFFIX", data["suffix"], gh_env_file)
    add_env("WORKFLOW_BRANCH_OR_TAG", data["branch_name"], gh_env_file)
    add_set_output_var("branch_name", data["branch_name"], gh_out_file)
    add_set_output_var("commit_sha", data["commit_sha"], gh_out_file)
    add_set_output_var("default_target", os.getenv("DEFAULT_TARGET"), gh_out_file)
    add_set_output_var("suffix", data["suffix"], gh_out_file)
    if args.type == "pull":
        add_env("PULL_ID", data["pull_id"], gh_env_file)
        add_env("PULL_NAME", data["pull_name"], gh_env_file)


def main():
    args = parse_args()
    event_file = open(args.event_file, "r")
    event = json.load(event_file)
    gh_env_file = open(os.environ["GITHUB_ENV"], "a")
    gh_out_file = open(os.environ["GITHUB_OUTPUT"], "a")
    data = get_details(event, args)
    add_envs(data, gh_env_file, gh_out_file, args)
    event_file.close()
    gh_env_file.close()
    gh_out_file.close()


if __name__ == "__main__":
    main()
