# Apps Assets folder Example {#example_app_assets}

This example shows how to use the Apps Assets folder to store data that is not part of the application itself, but is required for its operation, and that data is provided with the application.

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_apps_assets).

## What is the Apps Assets Folder?

The **Apps Assets** folder is a folder where external applications unpack their assets.

The path to the current application folder is related to the `appid` of the app. The `appid` is used to identify the app in the app store and is stored in the `application.fam` file. 
The Apps Assets folder is located only on the external storage, the SD card.

For example, if the `appid` of the app is `snake_game`, the path to the Apps Assets folder will be `/ext/apps_assets/snake_game`. But using raw paths is not recommended, because the path to the Apps Assets folder can change in the future. Use the `/assets` alias instead.

## How to get the path to the Apps Assets folder?

You can use `/assets` alias to get the path to the current application data folder. For example, if you want to open a file `database.txt` in the Apps Assets folder, you can use the next path: `/data/database.txt`. But this way is not recommended, because even the `/assets` alias can change in the future.

We recommend to use the `APP_ASSETS_PATH` macro to get the path to the Apps Assets folder. For example, if you want to open a file `database.txt` in the Apps Assets folder, you can use the next path: `APP_ASSETS_PATH("database.txt")`.

## What is the difference between the Apps Assets folder and the Apps Data folder?

The Apps Assets folder is used to store the data <u>provided</u> with the application. For example, if you want to create a game, you can store game levels (content data) in the Apps Assets folder.

The Apps Data folder is used to store data <u>generated</u> by the application. For example, if you want to create a game, you can save the progress of the game (user-generated data) in the Apps Data folder.

## How to provide the data with the app?

To provide data with an application, you need to create a folder inside your application folder (eg "files") and place the data in it. After that, you need to add `fap_file_assets="files"` to your application.fam file.

For example, if you want to provide game levels with the application, you need to create a "levels" folder inside the "files" folder and put the game levels in it. After that, you need to add `fap_file_assets="files"` to your application.fam file. The final application folder structure will look like this:

```
snake_game
├── application.fam
├── snake_game.c
└── files
    └── levels
        ├── level1.txt
        ├── level2.txt
        └── level3.txt
```

When app is launched, the `files` folder will be unpacked to the Apps Assets folder. The final structure of the Apps Assets folder will look like this:

```
/assets
├── .assets.signature
└── levels
    ├── level1.txt
    ├── level2.txt
    └── level3.txt
```

## When will the data be unpacked?

The data is unpacked when the application starts, if the application is launched for the first time, or if the data within the application is updated.

When an application is compiled, the contents of the "files" folder are hashed and stored within the application itself. When the application starts, this hash is compared to the hash stored in the `.assets.signature` file. If the hashes differ or the `.assets.signature` file does not exist, the application folder is deleted and the new data is unpacked.
