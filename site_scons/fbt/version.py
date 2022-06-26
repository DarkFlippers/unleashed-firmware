import subprocess
import datetime


def get_fast_git_version_id():
    try:
        version = (
            subprocess.check_output(
                [
                    "git",
                    "describe",
                    "--always",
                    "--dirty",
                    "--all",
                    "--long",
                ]
            )
            .strip()
            .decode()
        )
        return (version, datetime.date.today())
    except Exception as e:
        print("Failed to check for git changes", e)
