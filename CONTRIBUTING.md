# Welcome to FlipperZero contributing guide <!-- omit in toc -->

Thank you for investing your time in contributing to our project! 

Read our [Code of Conduct](CODE_OF_CONDUCT.md) to keep our community approachable and respectable.

In this guide you will get an overview of the contribution workflow from opening an issue, creating a PR, reviewing, and merging the PR.

## New contributor guide

See the [ReadMe](ReadMe.md) to get an overview of the project. Here are some helpful resources to get you comfortable with open source contribution:

- [Finding ways to contribute to open source on GitHub](https://docs.github.com/en/get-started/exploring-projects-on-github/finding-ways-to-contribute-to-open-source-on-github)
- [Set up Git](https://docs.github.com/en/get-started/quickstart/set-up-git)
- [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow)
- [Collaborating with pull requests](https://docs.github.com/en/github/collaborating-with-pull-requests)

## Getting started

Before writing code and creating PR make sure that it aligns with our mission and guidelines:

- All our devices are intended for research and education.
- PR that contains code intended to commit crimes is not going to be accepted.
- Your PR must comply with our [Coding Style](CODING_STYLE.md)
- Your PR must contain code compatible with project [LICENSE](LICENSE).
- PR will only be merged if it passes CI/CD.
- PR will only be merged if it passes review by code owner.

Feel free to ask questions in issues if you're not sure.

### Issues

#### Create a new issue

If you found a problem, [search if an issue already exists](https://docs.github.com/en/github/searching-for-information-on-github/searching-on-github/searching-issues-and-pull-requests#search-by-the-title-body-or-comments). If a related issue doesn't exist, you can open a new issue using a relevant [issue form](https://github.com/flipperdevices/flipperzero-firmware/issues/new/choose). 

#### Solve an issue

Scan through our [existing issues](https://github.com/flipperdevices/flipperzero-firmware/issues) to find one that interests you.

### Make Changes

1. Fork the repository.
- Using GitHub Desktop:
  - [Getting started with GitHub Desktop](https://docs.github.com/en/desktop/installing-and-configuring-github-desktop/getting-started-with-github-desktop) will guide you through setting up Desktop.
  - Once Desktop is set up, you can use it to [fork the repo](https://docs.github.com/en/desktop/contributing-and-collaborating-using-github-desktop/cloning-and-forking-repositories-from-github-desktop)!

- Using the command line:
  - [Fork the repo](https://docs.github.com/en/github/getting-started-with-github/fork-a-repo#fork-an-example-repository) so that you can make your changes without affecting the original project until you're ready to merge them.

2. Install build requirements

3. Create a working branch and start with your changes!

### Commit your update

Commit the changes once you are happy with them. Make sure that code compilation is not broken and passes tests. Check syntax and formatting.

### Pull Request

When you're done making the changes, open a pull request, often referred to as a PR. 
- Fill out the "Ready for review" template, so we can review your PR. This template helps reviewers understand your changes and the purpose of your pull request. 
- Don't forget to [link PR to issue](https://docs.github.com/en/issues/tracking-your-work-with-issues/linking-a-pull-request-to-an-issue) if you are solving one.
- Enable the checkbox to [allow maintainer edits](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/allowing-changes-to-a-pull-request-branch-created-from-a-fork) so the branch can be updated for a merge.
Once you submit your PR, a Docs team member will review your proposal. We may ask questions or request for additional information.
- We may ask for changes to be made before a PR can be merged, either using [suggested changes](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/incorporating-feedback-in-your-pull-request) or pull request comments. You can apply suggested changes directly through the UI. You can make any other changes in your fork, then commit them to your branch.
- As you update your PR and apply changes, mark each conversation as [resolved](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/commenting-on-a-pull-request#resolving-conversations).
- If you run into any merge issues, checkout this [git tutorial](https://lab.github.com/githubtraining/managing-merge-conflicts) to help you resolve merge conflicts and other issues.

### Your PR is merged!

Congratulations :tada::tada: The FlipperDevices team thanks you :sparkles:.
