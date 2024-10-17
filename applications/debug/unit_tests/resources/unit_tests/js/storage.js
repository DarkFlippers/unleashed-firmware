let storage = require("storage");
let tests = require("tests");

let baseDir = "/ext/.tmp/unit_tests";

tests.assert_eq(true, storage.rmrf(baseDir));
tests.assert_eq(true, storage.makeDirectory(baseDir));

// write
let file = storage.openFile(baseDir + "/helloworld", "w", "create_always");
tests.assert_eq(true, !!file);
tests.assert_eq(true, file.isOpen());
tests.assert_eq(13, file.write("Hello, World!"));
tests.assert_eq(true, file.close());
tests.assert_eq(false, file.isOpen());

// read
file = storage.openFile(baseDir + "/helloworld", "r", "open_existing");
tests.assert_eq(true, !!file);
tests.assert_eq(true, file.isOpen());
tests.assert_eq(13, file.size());
tests.assert_eq("Hello, World!", file.read("ascii", 128));
tests.assert_eq(true, file.close());
tests.assert_eq(false, file.isOpen());

// seek
file = storage.openFile(baseDir + "/helloworld", "r", "open_existing");
tests.assert_eq(true, !!file);
tests.assert_eq(true, file.isOpen());
tests.assert_eq(13, file.size());
tests.assert_eq("Hello, World!", file.read("ascii", 128));
tests.assert_eq(true, file.seekAbsolute(1));
tests.assert_eq(true, file.seekRelative(2));
tests.assert_eq(3, file.tell());
tests.assert_eq(false, file.eof());
tests.assert_eq("lo, World!", file.read("ascii", 128));
tests.assert_eq(true, file.eof());
tests.assert_eq(true, file.close());
tests.assert_eq(false, file.isOpen());

// byte-level copy
let src = storage.openFile(baseDir + "/helloworld", "r", "open_existing");
let dst = storage.openFile(baseDir + "/helloworld2", "rw", "create_always");
tests.assert_eq(true, !!src);
tests.assert_eq(true, src.isOpen());
tests.assert_eq(true, !!dst);
tests.assert_eq(true, dst.isOpen());
tests.assert_eq(true, src.copyTo(dst, 10));
tests.assert_eq(true, dst.seekAbsolute(0));
tests.assert_eq("Hello, Wor", dst.read("ascii", 128));
tests.assert_eq(true, src.copyTo(dst, 3));
tests.assert_eq(true, dst.seekAbsolute(0));
tests.assert_eq("Hello, World!", dst.read("ascii", 128));
tests.assert_eq(true, src.eof());
tests.assert_eq(true, src.close());
tests.assert_eq(false, src.isOpen());
tests.assert_eq(true, dst.eof());
tests.assert_eq(true, dst.close());
tests.assert_eq(false, dst.isOpen());

// truncate
tests.assert_eq(true, storage.copy(baseDir + "/helloworld", baseDir + "/helloworld2"));
file = storage.openFile(baseDir + "/helloworld2", "w", "open_existing");
tests.assert_eq(true, !!file);
tests.assert_eq(true, file.seekAbsolute(5));
tests.assert_eq(true, file.truncate());
tests.assert_eq(true, file.close());
file = storage.openFile(baseDir + "/helloworld2", "r", "open_existing");
tests.assert_eq(true, !!file);
tests.assert_eq("Hello", file.read("ascii", 128));
tests.assert_eq(true, file.close());

// existence
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld"));
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld2"));
tests.assert_eq(false, storage.fileExists(baseDir + "/sus_amogus_123"));
tests.assert_eq(false, storage.directoryExists(baseDir + "/helloworld"));
tests.assert_eq(false, storage.fileExists(baseDir));
tests.assert_eq(true, storage.directoryExists(baseDir));
tests.assert_eq(true, storage.fileOrDirExists(baseDir));
tests.assert_eq(true, storage.remove(baseDir + "/helloworld2"));
tests.assert_eq(false, storage.fileExists(baseDir + "/helloworld2"));

// stat
let stat = storage.stat(baseDir + "/helloworld");
tests.assert_eq(true, !!stat);
tests.assert_eq(baseDir + "/helloworld", stat.path);
tests.assert_eq(false, stat.isDirectory);
tests.assert_eq(13, stat.size);

// rename
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld"));
tests.assert_eq(false, storage.fileExists(baseDir + "/helloworld123"));
tests.assert_eq(true, storage.rename(baseDir + "/helloworld", baseDir + "/helloworld123"));
tests.assert_eq(false, storage.fileExists(baseDir + "/helloworld"));
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld123"));
tests.assert_eq(true, storage.rename(baseDir + "/helloworld123", baseDir + "/helloworld"));
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld"));
tests.assert_eq(false, storage.fileExists(baseDir + "/helloworld123"));

// copy
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld"));
tests.assert_eq(false, storage.fileExists(baseDir + "/helloworld123"));
tests.assert_eq(true, storage.copy(baseDir + "/helloworld", baseDir + "/helloworld123"));
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld"));
tests.assert_eq(true, storage.fileExists(baseDir + "/helloworld123"));

// next avail
tests.assert_eq("helloworld1", storage.nextAvailableFilename(baseDir, "helloworld", "", 20));

// fs info
let fsInfo = storage.fsInfo("/ext");
tests.assert_eq(true, !!fsInfo);
tests.assert_eq(true, fsInfo.freeSpace < fsInfo.totalSpace); // idk \(-_-)/
fsInfo = storage.fsInfo("/int");
tests.assert_eq(true, !!fsInfo);
tests.assert_eq(true, fsInfo.freeSpace < fsInfo.totalSpace);

// path operations
tests.assert_eq(true, storage.arePathsEqual("/ext/test", "/ext/Test"));
tests.assert_eq(false, storage.arePathsEqual("/ext/test", "/ext/Testttt"));
tests.assert_eq(true, storage.isSubpathOf("/ext/test", "/ext/test/sub"));
tests.assert_eq(false, storage.isSubpathOf("/ext/test/sub", "/ext/test"));

// dir
let entries = storage.readDirectory(baseDir);
tests.assert_eq(true, !!entries);
// FIXME: (-nofl) this test suite assumes that files are listed by
// `readDirectory` in the exact order that they were created, which is not
// something that is actually guaranteed.
// Possible solution: sort and compare the array.
tests.assert_eq("helloworld", entries[0].path);
tests.assert_eq("helloworld123", entries[1].path);

tests.assert_eq(true, storage.rmrf(baseDir));
tests.assert_eq(true, storage.makeDirectory(baseDir));
