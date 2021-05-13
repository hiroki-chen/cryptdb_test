# Guidance and Notes
This is a tester for FHCrypt_DB which is also available on my Github repositories. To use this, please read me.

## Preliminaries
* Install *MySQL-connector-c++* libraries (available on [this page](https://downloads.mysql.com/archives/c-cpp/)); </br>
  For macOS users, you can install it by HomeBrew, using the command as follows.
  ```shell
  brew install mysql-connector-c++
  ```
  and then modify the mysql-connector-c++ libraries' absolute path in Makefile.
  ```shell
  MYSQL_CONCPP_DIR=</path/to/your/connector/library> make test
  ```
  For macOS users, the dynamic libraries may not be directly linked to the system and running the program will thus throw some errors. Please use `ln -s` command to link these
  *dylib*s to your system.
* Specify your compiler (either GNU compiler or clang++);
* Make sure they support c++ 17;
* Have MySQL installed; furthermore, you should specify `secure_file_priv` and specify it in the source file as well (if you do not want to perform attacks, just leave it).
* Compile `ope.cc` and make it a shared object, namely, `ope.so` and move it to the plugin directory of MySQL. Then, run command
  ```shell
  mysql> source <sql/udf.sql>;
  ```
  which will automatically install all the needed user defined functions.
 * Edit the file `input/test_case.txt` as you wish. Rules:
   ```
   <type = OPE | DET>, <csv column name>, <the number of plaintexts to be encrypted>, <params...>
   ```
 * You could also specify some arguments like the username, port, file_path, or the server address; they can be edited in `test/test.cc` file.
