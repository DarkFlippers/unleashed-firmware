#!/usr/bin/env python3

from datetime import datetime
import argparse
import mariadb
import sys
import os


def parseArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument("db_user", help="MariaDB user")
    parser.add_argument("db_pass", help="MariaDB password")
    parser.add_argument("db_host", help="MariaDB hostname")
    parser.add_argument("db_port", type=int, help="MariaDB port")
    parser.add_argument("db_name", help="MariaDB database")
    parser.add_argument("report_file", help="Report file(.map.all)")
    args = parser.parse_args()
    return args


def mariadbConnect(args):
    try:
        conn = mariadb.connect(
            user=args.db_user,
            password=args.db_pass,
            host=args.db_host,
            port=args.db_port,
            database=args.db_name,
        )
    except mariadb.Error as e:
        print(f"Error connecting to MariaDB: {e}")
        sys.exit(1)
    return conn


def parseEnv():
    outArr = []
    outArr.append(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    outArr.append(os.getenv("COMMIT_HASH", default=None))
    outArr.append(os.getenv("COMMIT_MSG", default=None))
    outArr.append(os.getenv("BRANCH_NAME", default=None))
    outArr.append(os.getenv("BSS_SIZE", default=None))
    outArr.append(os.getenv("TEXT_SIZE", default=None))
    outArr.append(os.getenv("RODATA_SIZE", default=None))
    outArr.append(os.getenv("DATA_SIZE", default=None))
    outArr.append(os.getenv("FREE_FLASH_SIZE", default=None))
    outArr.append(os.getenv("PULL_ID", default=None))
    outArr.append(os.getenv("PULL_NAME", default=None))
    return outArr


def createTables(cur, conn):
    headerTable = "CREATE TABLE IF NOT EXISTS `header` ( \
            `id` int(10) unsigned NOT NULL AUTO_INCREMENT, \
            `datetime` datetime NOT NULL, \
            `commit` varchar(40) NOT NULL, \
            `commit_msg` text NOT NULL, \
            `branch_name` text NOT NULL, \
            `bss_size` int(10) unsigned NOT NULL, \
            `text_size` int(10) unsigned NOT NULL, \
            `rodata_size` int(10) unsigned NOT NULL, \
            `data_size` int(10) unsigned NOT NULL, \
            `free_flash_size` int(10) unsigned NOT NULL, \
            `pullrequest_id` int(10) unsigned DEFAULT NULL, \
            `pullrequest_name` text DEFAULT NULL, \
            PRIMARY KEY (`id`), \
            KEY `header_id_index` (`id`) )"
    dataTable = "CREATE TABLE IF NOT EXISTS `data` ( \
            `header_id` int(10) unsigned NOT NULL, \
            `id` int(10) unsigned NOT NULL AUTO_INCREMENT, \
            `section` text NOT NULL, \
            `address` text NOT NULL, \
            `size` int(10) unsigned NOT NULL, \
            `name` text NOT NULL, \
            `lib` text NOT NULL, \
            `obj_name` text NOT NULL, \
            PRIMARY KEY (`id`), \
            KEY `data_id_index` (`id`), \
            KEY `data_header_id_index` (`header_id`), \
            CONSTRAINT `data_header_id_foreign` FOREIGN KEY (`header_id`) REFERENCES `header` (`id`) )"
    cur.execute(headerTable)
    cur.execute(dataTable)
    conn.commit()


def insertHeader(data, cur, conn):
    query = "INSERT INTO `header` ( \
            datetime, commit, commit_msg, branch_name, bss_size, text_size, \
            rodata_size, data_size, free_flash_size, pullrequest_id, pullrequest_name) \
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    cur.execute(query, data)
    conn.commit()
    return cur.lastrowid


def parseFile(fileObj, headerID):
    arr = []
    fileLines = fileObj.readlines()
    for line in fileLines:
        lineArr = []
        tempLineArr = line.split("\t")
        lineArr.append(headerID)
        lineArr.append(tempLineArr[0])  # section
        lineArr.append(int(tempLineArr[2], 16))  # address hex
        lineArr.append(int(tempLineArr[3]))  # size
        lineArr.append(tempLineArr[4])  # name
        lineArr.append(tempLineArr[5])  # lib
        lineArr.append(tempLineArr[6])  # obj_name
        arr.append(tuple(lineArr))
    return arr


def insertData(data, cur, conn):
    query = "INSERT INTO `data` ( \
            header_id, section, address, size, \
            name, lib, obj_name) \
            VALUES (?, ?, ?, ?, ? ,?, ?)"
    cur.executemany(query, data)
    conn.commit()


def main():
    args = parseArgs()
    dbConn = mariadbConnect(args)
    reportFile = open(args.report_file)
    dbCurs = dbConn.cursor()
    createTables(dbCurs, dbConn)
    headerID = insertHeader(parseEnv(), dbCurs, dbConn)
    insertData(parseFile(reportFile, headerID), dbCurs, dbConn)
    reportFile.close()
    dbCurs.close()


if __name__ == "__main__":
    main()
