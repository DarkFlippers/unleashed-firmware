# Welcome to the Flipper Zero contribution guide <!-- omit in toc -->

Thank you for investing your time in contributing to our project! 

Read our [Code of Conduct](CODE_OF_CONDUCT.md) to keep our community approachable and respectable.

In this guide you will get an overview of the contribution workflow from opening an issue, creating a PR, reviewing, and merging PRs.

## New to open source?

See the [ReadMe](ReadMe.md) file to get an overview of the project. Also, read these helpful resources to get you comfortable with open source contribution:

- [Ways to contribute to open source on GitHub](https://docs.github.com/en/get-started/exploring-projects-on-github/finding-ways-to-contribute-to-open-source-on-github)
- [Set up Git](https://docs.github.com/en/get-started/quickstart/set-up-git)
- [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow)
- [Collaborating with pull requests](https://docs.github.com/en/github/collaborating-with-pull-requests)

## How to contribute

### Step 1. Review the guidelines

Before writing code and creating a PR, make sure your contribution aligns with our mission and guidelines:

- All our devices are intended for research and education.
- We won't accept code intended to commit crimes.
- Follow our [Coding Style](CODING_STYLE.md).
- Use code compatible with the project [License](LICENSE).
- Your PR will only be merged if it passes CI/CD.
- A code owner must approve the PR.
- AI-generated PRs (especially changes in core parts like FURI, FreeRTOS, Libs, SDK) will be reviewed separately from others. PRs that do not provide sufficient value to the project may be rejected by maintainers or remain in draft.

Feel free to ask questions in [issues](https://github.com/flipperdevices/flipperzero-firmware/issues) or [discussions](https://github.com/flipperdevices/flipperzero-firmware/discussions) if you're not sure. For feature requests and voting, follow our [Discussion Guidelines](https://github.com/flipperdevices/flipperzero-firmware/discussions/4395).


### Step 2. Check for an existing issue

Before you start working on something, check if this problem was already reported. [Search the existing issues](https://github.com/flipperdevices/flipperzero-firmware/issues) first to avoid duplicating effort. Here is more info on [how to search issues](https://docs.github.com/en/search-github/searching-on-github/searching-issues-and-pull-requests#search-only-issues-or-pull-requests).

If an issue doesn't exist, you can open a new one using a relevant [issue form](https://github.com/flipperdevices/flipperzero-firmware/issues/new/choose).

### Step 3. Fork the repo and set up the project

  1. **[Fork the repository](https://github.com/flipperdevices/flipperzero-firmware/fork)** to your own GitHub account. This creates a copy you can push to freely. (New to forking? See GitHub's [fork a repo](https://docs.github.com/en/get-started/quickstart/fork-a-repo) guide, or do it from [GitHub Desktop](https://docs.github.com/en/desktop/contributing-and-collaborating-using-github-desktop/cloning-and-forking-repositories-from-github-desktop).)

  2. **Clone your fork** to your computer. Use `--recursive` so all submodules are fully loaded:
  ```
  git clone --recursive https://github.com/YOUR-USERNAME/flipperzero-firmware.git
  ```

  3. **Create a working branch.** Use a descriptive name such as `feature/your-change` then start making your changes.

  4. **Install build requirements and build** with the Flipper Build Tool (FBT). See [FBT documentation](documentation/fbt.md) for details. Basic commands:

  - `./fbt` — build firmware with debug flags enabled.
  - `./fbt COMPACT=1 DEBUG=0 updater_package` — build full updater package `.tgz` with release flags.
  - `./fbt vscode_dist` — add VS Code environment to the project.
  - `./fbt format` — format code in the project folder (apply this before making a PR).


### Step 4. Make changes
Implement the changes to the firmware code, following the [Coding Style](CODING_STYLE.md) guidelines.


### Step 5. Commit changes

Commit the changes once you are happy with them. Make sure code compilation succeeds, tests pass, and formatting follows [Coding Style](CODING_STYLE.md) and `./fbt format` is applied. Use clear, descriptive commit messages.

### Step 6. Create a pull request

When you're done making the changes, open a pull request:
- Fill out the "Ready for review" template, so we can review your PR. This template helps reviewers understand your changes and the purpose of your pull request. 
- Don't forget to [link PR to issue](https://docs.github.com/en/issues/tracking-your-work-with-issues/linking-a-pull-request-to-an-issue) if you are solving one.
- Make sure the [Allow edits from maintainers](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/allowing-changes-to-a-pull-request-branch-created-from-a-fork) checkbox is checked so the branch can be updated before merging.

### Step 7. Respond to review

- We may ask for changes to be made before a PR can be merged, either using [suggested changes](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/incorporating-feedback-in-your-pull-request) or pull request comments. You can apply suggested changes directly through the UI. You can make any other changes in your fork, then commit them to your branch.
- As you update your PR and apply changes, mark each conversation as [resolved](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/commenting-on-a-pull-request#resolving-conversations).
- If you run into any merge issues, check out this [tutorial](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/addressing-merge-conflicts/resolving-a-merge-conflict-on-github) to help you resolve merge conflicts and other issues.

### Your PR is merged!

Congratulations :tada:! The Flipper Devices team thanks you for your contribution :sparkles:
