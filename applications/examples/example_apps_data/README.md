# Apps Data folder Example

This example demonstrates how to utilize the Apps Data folder to store data that is not part of the app itself, such as user data, configuration files, and so forth.

## What is the Apps Data Folder?

The **Apps Data** folder is a folder used to store data for external apps that are not part of the main firmware. 

The path to the current application folder is related to the `appid` of the app. The `appid` is used to identify the app in the app store and is stored in the `application.fam` file. 
The Apps Data folder is located only on the external storage, the SD card.

For example, if the `appid` of the app is `snake_game`, the path to the Apps Data folder will be `/ext/apps_data/snake_game`. But using raw paths is not recommended, because the path to the Apps Data folder can change in the future. Use the `/app` alias instead.

## How to get the path to the Apps Data folder?

You can use `/app` alias to get the path to the current application data folder. For example, if you want to open a file `config.txt` in the Apps Data folder, you can use the next path: `/app/config.txt`. But this way is not recommended, because even the `/app` alias can change in the future.

We recommend to use the `APP_DATA_PATH` macro to get the path to the Apps Data folder. For example, if you want to open a file `config.txt` in the Apps Data folder, you can use the next path: `APP_DATA_PATH("config.txt")`.