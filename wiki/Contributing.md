([see issue about this page](https://github.com/Flipper-Zero/flipperzero-firmware-community/labels/Area%3AContribution-guide))

# Getting Started

If you are just beginning in Flipper, **Read the [wiki](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki)**. It describes general things like contribution, building and testing, and tell about main features. Flipper consists of two main parts:

* Core: OS, HAL, FS, bootloader, FURI
* Applications: features like RFID or Tamagotchi, and also background tasks like button debouncing and control the backlight.

## General Tips and principles

* **Ask around for help!** If you have any questions, feel free to create an [`need help` issue](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/new?assignees=&labels=need+help&template=need-help.md&title=) or send an email to devel@flipperdevices.com. The earlier you check your feature design with other people, the less likely it is that it is denied during the review process.
* **Verify your concept early!** If you work on your own until the code looks good enough to show publicly, you might miss some design flaws others might have spotted earlier.
* **Keep it simple!** Try to use what is already there and don't change existing APIs if not absolutely necessary.
* **State your intentions** Create issue before you start your work. It will prevent a very frustrating situation where several people are doing the same job the same time.
* **Make tests `(incomplete)`**
* **Make docs** you can do very cool things but other people cannot use if it is not described in the documentation
* **We are open to changes.** You can suggest changes to any part of the code, wiki, guidelines, workflow, automation, etc. by creating issue or PR if you understand how it can be done better. 

## Status of wiki sections

* Some sections mark as `incomplete`. This means that there is not even a description of how the feature can be implemented. You can start [discussion](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/new?assignees=&labels=discussion&template=discuss-issue.md&title=) thread or directly begin to write wiki page (see [Contributing](#contributing)).
* Some sections mark as `not implemented`. It have description but there is nothing that would make this description into reality.
* Some sections mark as `not documented`. There is some implementation and you can add some documentation here.
* If section of wiki has no mark, this is actual documentation for part of Flipper.

## Wiki editing `(not implemented)`

All wiki files storage in main repository in `wiki` folder. You can change wiki by creating PR with `documentation` label. After merge to master, wiki contents copying to GitHub `Wiki` section.

### For maintainers

If you want to update wiki:

* Do not edit wiki directly on github!
* place `flipperzero-firmware-community.wiki` folder repo in main repo's root folder (do not add it to git!) 
* call `./wiki-deploy.sh` srcipt

# Maintainers

Every section/page of wiki and related part of code has its own maintainers. Maintainers list and related code folders is placed at the bottom of page or section.

If contributors cannot reach consensus during a discussion or code review, you can add a [needs moderating](https://github.com/Flipper-Zero/flipperzero-firmware-community/labels/needs%20moderating) label. Maintainers has the final say in the discussion.

If maintainers cannot reach consensus, Flipper devices CTO ([`@glitchcore`](https://github.com/glitchcore)) has the final say in the discussion.

# Issues

Please notice that we use a bunch of tags to label the issues.

All issues are, if possible, tied to sections in the wiki with `Area:<wiki page name>` labels.

Full list of labels you can find at [labels list page](https://github.com/Flipper-Zero/flipperzero-firmware-community/labels).

# Contributing

If you want to add some features or suggest some changes, do following steps:

1. Choose section which you want to improve
2. Check existing issues and PR by `Area` tag. Maybe somebody create discussion about your improvement or already doing some work.
3. Choose your way:
	* If you have some idea about new feature, you can create **[feature request](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/new?assignees=&labels=feature+request&template=feature_request.md&title=)**
	* If you find some bug, you can create **[bug report](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/new?assignees=&labels=bug&template=bug_report.md&title=)**
	* You can ask for help if you are not sure how to implement your idea or discuss implementation details by creating **[discuss issue](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/new?assignees=&labels=discussion&template=discuss-issue.md&title=)**.
4. Otherwise, make you improvement:
	* Clone actual repository.
	* Create a branch
	* Make commits
	* Push your branch and create a [pull request](#pull-requests)
	* Wait for maintainers feedback.
	* Your code is merged in master branch
5. If you can do only part of work, create PR with `WIP` label. Describe what you have already done and what remains to be done and other people can help you.

## Pull requests

1. Don't forget reference issues or other PR
2. Add `Area:` label to PR
3. Remember that smaller PRs tend to be merged faster, so keep your changes as concise as possible. They should be confined to a single explainable change, and be runnable on their own. So don't hesitate to split your PRs into smaller ones when possible.
4. If you only create description at wiki but no implementation existing, don't forget to add `not implemented` mark.
5. We strongly recommend documenting your code and creating wiki descriptions at the same time as improvement the code. If you have no energy left for documentation, at least mark the appropriate section of the wiki as `not documented`, or you can create `WIP` PR and wait for help.

### Large and static files storage (incomplete)

_Maintainers of this page: [`@glitchcore`](https://github.com/glitchcore)_