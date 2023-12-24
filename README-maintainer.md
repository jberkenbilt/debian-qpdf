# Maintainer Notes

## Contents

* [ROUTINE DEVELOPMENT](#routine-development)
* [VERSIONS](#versions)
* [CHECKING DOCS ON readthedocs](#checking-docs-on-readthedocs)
* [GOOGLE OSS-FUZZ](#google-oss-fuzz)
* [CODING RULES](#coding-rules)
* [ZLIB COMPATIBILITY](#zlib-compatibility)
* [HOW TO ADD A COMMAND-LINE ARGUMENT](#how-to-add-a-command-line-argument)
* [RELEASE PREPARATION](#release-preparation)
* [CREATING A RELEASE](#creating-a-release)
* [RUNNING pikepdf's TEST SUITE](#running-pikepdfs-test-suite)
* [OTHER NOTES](#other-notes)
* [DEPRECATION](#deprecation)
* [LOCAL WINDOWS TESTING PROCEDURE](#local-windows-testing-procedure)
* [DOCS ON readthedocs.org](#docs-on-readthedocsorg)
* [CMAKE notes](#cmake-notes)
* [ABI checks](#abi-checks)
* [CODE FORMATTING](#code-formatting)


## ROUTINE DEVELOPMENT

**Remember to check pull requests as well as issues in github.**

Default:

```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
   -DMAINTAINER_MODE=1 -DBUILD_STATIC_LIBS=0 \
   -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

Debugging:

```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
   -DMAINTAINER_MODE=1 -DBUILD_SHARED_LIBS=0 \
   -DCMAKE_BUILD_TYPE=Debug ..
```

Profiling:

```
CFLAGS=-pg LDFLAGS=-pg \
   cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
   -DMAINTAINER_MODE=1 -DBUILD_SHARED_LIBS=0 \
   -DCMAKE_BUILD_TYPE=Debug ..
```

Then run `gprof gmon.out`. Note that gmon.out is not cumulative.

Memory checks:

```
CFLAGS="-fsanitize=address -fsanitize=undefined" \
   CXXFLAGS="-fsanitize=address -fsanitize=undefined" \
   LDFLAGS="-fsanitize=address -fsanitize=undefined" \
   CC=clang CXX=clang++ \
   cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
   -DMAINTAINER_MODE=1 -DBUILD_SHARED_LIBS=0 \
   -DCMAKE_BUILD_TYPE=Debug ..
```

Windows:

```
../cmake-win {mingw|msvc} maint
```

See ./build-scripts for other ways to run the build for different
configurations.

## VERSIONS

* The version number on the main branch is whatever the version would
  be if the top of the branch were released. If the most recent
  release is version a.b.c, then

  * If there are any ABI-breaking changes since the last release,
    main's version is a+1.0.0
  * Else if there is any new public API, main's version is a.b+1.0
  * Else if there are any code changes, main's version is a.b.c+1

* Whenever we bump the version number, bump shared library versions as
  well.

* Every released major/minor version has an a.b branch which is used
  primarily for documentation but could potentially be used to create
  a new patch release after main has moved on. We don't do that as a
  rule, but there's no reason we couldn't do it if main had unreleased
  ABI/API changes that were still in flux and an important bug fix was
  needed on the most recent release. In that case, a release can be
  cut from a release branch and then either main can be rebased from
  there or the changes can be merged back, depending on the amount of
  drift.

## CHECKING DOCS ON readthedocs

To check docs on readthedocs.io without running all of CI, push to the
doc-check branch. Then visit https://qpdf.readthedocs.io/en/doc-check/
Building docs from pull requests is also enabled.

## GOOGLE OSS-FUZZ

* See ../misc/fuzz (not in repo) for unfixed, downloaded fuzz test cases

* qpdf project: https://github.com/google/oss-fuzz/tree/master/projects/qpdf

* Adding new test cases: download the file from oss-fuzz and drop it
  in fuzz/qpdf_extra/issue-number.fuzz. When ready to include it, add
  to fuzz/CMakeLists.txt. Until ready to use, the file can be stored
  anywhere, and the absolute path can be passed to the reproduction
  code as described below.

* To test locally, see https://github.com/google/oss-fuzz/tree/master/docs/,
  especially new_project_guide.md. Summary:

  Clone the oss-fuzz project. From the root directory of the repository:

  ```
  python3 infra/helper.py build_image --pull qpdf
  python3 infra/helper.py build_fuzzers [ --sanitizer memory|undefined|address ] qpdf [path-to-qpdf-source]
  python3 infra/helper.py check_build qpdf
  python3 infra/helper.py build_fuzzers --sanitizer coverage qpdf
  python3 infra/helper.py coverage qpdf
  ```

  To reproduce a test case, build with the correct sanitizer, then run

  python3 infra/helper.py reproduce qpdf <specific-fuzzer> testcase

  where fuzzer is the fuzzer used in the crash.

  The fuzzer is in build/out/qpdf. It can be run with a directory as
  an argument to run against files in a directory. You can use

  qpdf_fuzzer -merge=1 cur new >& /dev/null&

  to add any files from new into cur if they increase coverage. You
  need to do this with the coverage build (the one with
  --sanitizer coverage)

* General documentation: http://libfuzzer.info

* Build status: https://oss-fuzz-build-logs.storage.googleapis.com/index.html

* Project status: https://oss-fuzz.com/ (private -- log in with Google account)

* Latest corpus:
  gs://qpdf-backup.clusterfuzz-external.appspot.com/corpus/libFuzzer/qpdf_fuzzer/latest.zip

## CODING RULES

* Code is formatted with clang-format-16. See .clang-format and the
  "Code Formatting" section in manual/contributing.rst for details.
  See also "CODE FORMATTING" below.

* Use std::to_string instead of QUtil::int_to_string et al

* Use of assert:

  * Test code: #include <qpdf/assert_test.h> first.
  * Debug code: #include <qpdf/assert_debug.h> first and use
    qpdf_assert_debug instead of assert.

  These rules are enforced by the check-assert test. This practices
  serves to

  * remind us that assert in release code disappears and so should only
    be used for debugging; when doing so use a Debug build
    configuration

  * protect us from using assert in test code without explicitly
    removing the NDEBUG definition, since that would cause the assert
    not to actually be testing anything in non-Debug build
    configurations.

* In a source file, include the header file that declares the source
  class first followed by a blank line. If a config file is needed
  first, put a blank line between that and the header followed by
  another blank line. This assures that each header file is included
  first at least once, thereby ensuring that it explicitly includes
  all the headers it needs, which in turn alleviates lots of header
  ordering problems. The blank line ensures that formatters don't
  mess this up by resorting the headers.

* Avoid atoi. Use QUtil::string_to_int instead. It does
  overflow/underflow checking.

* Avoid certain functions that tend to be macros or create compilation
  errors on some platforms. Known cases: strcasecmp, abs. Avoid min
  and max. If needed, std::min and std::max are okay to use in C++
  code with <algorithm> included.

* Remember to avoid using `operator[]` with `std::string` or
  `std::vector`. Instead, use `at()`. See README-hardening.md for
  details.

* Use QIntC for type conversions -- see casting policy in docs.

* Remember to imbue ostringstreams with std::locale::classic() before
  outputting numbers. This protects against the user's global locale
  altering otherwise deterministic values. (See github issue #459.)
  One could argue that error messages containing numbers should
  respect the user's locale, but I think it's more important for
  output to be consistent, since the messages in question are not
  really targeted at the end user.

* Use QPDF_DLL on all methods that are to be exported in the shared
  library/DLL. Use QPDF_DLL_CLASS for all classes whose type
  information is needed. This is important for classes that are used
  as exceptions, subclassed, or tested with dynamic_cast across the
  the shared object boundary (or "shared library boundary" -- we may
  use either term in comments and documentation). In particular,
  anything new derived from Pipeline or InputSource should be marked
  with QPDF_DLL_CLASS. We shouldn't need to do it for QPDFObjectHelper
  or QPDFDocumentHelper subclasses since there's no reason to use
  dynamic_cast with those, but doing it anyway may help with some
  strange cases for mingw or with some code generators that may
  systematically do this for other reasons.

  IMPORTANT NOTE ABOUT QPDF_DLL_CLASS: On mingw, the vtable for a
  class with some virtual methods and no pure virtual methods seems
  often (always?) not to be generated if the destructor is inline or
  declared with `= default`. Therefore, for any class that is intended
  to be used as a base class and doesn't contain any pure virtual
  methods, you must declare the destructor in the header without
  `= default` and provide a non-inline implementation in the source
  file. Add this comment to the implementation:

    ```cpp
    // Must be explicit and not inline -- see QPDF_DLL_CLASS in
    // README-maintainer
    ```

* Put private member variables in std::unique_ptr<Members> for all
  public classes. Forward declare Members in the header file and define
  Members in the implementation file. One of the major benefits of
  defining Members in the implementation file is that it makes it easier
  to use private classes as data members and simplifies the include order.
  Remember that Members must be fully defined before the destructor of the
  main class. For an example of this pattern see class JSONHandler.

  Exception: indirection through std::unique_ptr<Members> incurs an overhead,
  so don't do it for:
  * (especially private) classes that are copied a lot, like QPDFObjectHandle
    and QPDFObject.
  * classes that are a shared pointer to another class, such as QPDFObjectHandle
    or JSON.

  For exported classes that do not use the member pattern for performance
  reasons it is worth considering adding a std::unique_ptr to an empty Members
  class initialized to nullptr to give the flexibility to add data members
  without breaking the ABI.

  Note that, as of qpdf 11, many public classes use `std::shared_ptr`
  instead. Changing this to `std::unique_ptr` is ABI-breaking. If the
  class doesn't allow copying, we can switch it to std::unique_ptr and
  let that be the thing that prevents copying. If the intention is to
  allow the object to be copied by value and treated as if it were
  copied by reference, then `std::shared_ptr<Members>` should be used.
  The `JSON` class is an example of this. As a rule, we should avoid
  this design pattern. It's better to make things non-copiable and to
  require explicit use of shared pointers, so going forward,
  `std::unique_ptr` should be preferred.

* Traversal of objects is expensive. It's worth adding some complexity
  to avoid needless traversals of objects.

* Avoid attaching too much metadata to objects and object handles
  since those have to get copied around a lot.

## ZLIB COMPATIBILITY

The qpdf test suite is designed to be independent of the output of any
particular version of zlib. There are several strategies to make this
work:

* `build-scripts/test-alt-zlib` runs in CI and runs the test suite
  with a non-default zlib. Please refer to that code for an example of
  how to do this in case you want to test locally.

* The test suite is full of cases that compare output PDF files with
  expected PDF files in the test suite. If the file contains data that
  was compressed by QPDFWriter, then the output file will depend on
  the behavior of zlib. As such, using a simple comparison won't work.
  There are several strategies used by the test suite.

  * A new program called `qpdf-test-compare`, in most cases, is a drop
    in replacement for a simple file comparison. This code make sure
    the two files have exactly the same number of objects with the
    same object and generation numbers, and that corresponding objects
    are identical with the following allowances (consult its source
    code for all the details details):
    * The `/Length` key is not compared in stream dictionaries.
    * The second element of `/ID` is not compared.
    * If the first and second element of `/ID` are the same, then the
      first element if `/ID` is also not compared.
    * If a stream is compressed with `/FlateDecode`, the
      _uncompressed_ stream data is compared. Otherwise, the raw
      stream data is compared.
    * Generated fields in the `/Encrypt` dictionary are not compared,
      though password-protected files must have the same password.
    * Differences in the contents of `/XRef` streams are ignored.

    To use this, run `qpdf-test-compare actual.pdf expected.pdf`, and
    expect the output to match `expected.pdf`. For example, if a test
    used to be written like this;
    ```perl
    $td->runtest("check output",
                 {$td->FILE => "a.pdf"},
                 {$td->FILE => "out.pdf"});
    ```
    then write it like this instead:
    ```perl
    $td->runtest("check output",
                 {$td->COMMAND => "qpdf-test-compare a.pdf out.pdf"},
                 {$td->FILE => "out.pdf", $td->EXIT_STATUS => 0});
    ```
    You can look at `compare-for-test/qtest/compare.test` for
    additional examples.

    Here's what's going on:
    * If the files "match" according to the rules of
      `qpdf-test-compare`, the output of the program is the expected
      file.
    * If the files do not match, the output is the actual file. The
      reason is that, if a change is made that results in an expected
      change to the expected file, the output of the comparison can be
      used to replace the expected file (as long as it is definitely
      known to be correct—no shortcuts here!). That way, it doesn't
      matter which zlib you use to generate test files.
    * As a special debugging tool, you can set the `QPDF_COMPARE_WHY`
      environment variable to any value. In this case, if the files
      don't match, the output is a description of the first thing in
      the file that doesn't match. This is mostly useful for debugging
      `qpdf-test-compare` itself, but it can also be helpful as a
      sanity check that the differences are expected. If you are
      trying to find out the _real_ differences, a suggestion is to
      convert both files to qdf and compare them lexically.

  * There are some cases where `qpdf-test-compare` can't be used. For
    example, if you need to actually test one of the things that
    `qpdf-test-compare` ignores, you'll need some other mechanism.
    There are tests for deterministic ID creation and xref streams
    that have to implement other mechanisms. Also, linearization hint
    streams and the linearization dictionary in a linearized file
    contain file offsets. Rather than ignoring those, it can be
    helpful to create linearized files using `--compress-streams=n`.
    In that case, `QPDFWriter` won't compress any data, so the PDF
    will be independent of the output of any particular zlib
    implementation.

You can find many examples of how tests were rewritten by looking at
the commits preceding the one that added this section of this README
file.

Note about `/ID`: many test cases use `--static-id` to have a
predictable `/ID` for testing. Many other test cases use
`--deterministic-id`. While `--static-id` is unaffected by file
contents, `--deterministic-id` is based on file contents and so is
dependent on zlib output if there is any newly compressed data. By
using `qpdf-test-compare`, it's actually not necessary to use either
`--static-id` or `--deterministic-id`. It may still be necessary to
use `--static-aes-iv` if comparing encrypted files, but since
`qpdf-test-compare` ignores `/Perms`, a wider range of encrypted files
can be compared using `qpdf-test-compare`.

## HOW TO ADD A COMMAND-LINE ARGUMENT

Quick reminder:

* Add an entry to the top half of job.yml for the command-line
  argument
* Add an entry to the bottom half of job.yml for the job JSON field
* Add documentation for the new option to cli.rst
* Implement the QPDFJob::Config method in QPDFJob_config.cc.

QPDFJob is documented in three places:

* This section provides a quick reminder for how to add a command-line
  argument

* generate_auto_job has a detailed explanation about how QPDFJob and
  generate_auto_job work together

* The manual ("QPDFJob Design" in qpdf-job.rst) discusses the design
  approach, rationale, and evolution of QPDFJob.

Command-line arguments are closely coupled with QPDFJob. To add a new
command-line argument, add the option to the appropriate table in
job.yml. This will automatically declare a method in the private
ArgParser class in QPDFJob_argv.cc which you have to implement. The
implementation should make calls to methods in QPDFJob via its Config
classes. Then, add the same option to either the no-json section of
job.yml if it is to be excluded from the job json structure, or add it
under the json structure to the place where it should appear in the
json structure.

In most cases, adding a new option will automatically declare and call
the appropriate Config method, which you then have to implement. If
you need a manual handler, you have to declare the option as manual in
job.yml and implement the handler yourself, though the automatically
generated code will declare it for you.

The build will fail until the new option is documented in
manual/cli.rst. To do that, create documentation for the option by
adding a ".. qpdf:option::" directive followed by a magic help comment
as described at the top of manual/cli.rst. Put this in the correct
help topic. Help topics roughly correspond with sections in that
chapter and are created using a special ".. help-topic" comment.
Follow the example of other options for style.

When done, the following should happen:

* qpdf --new-option should work as expected
* qpdf --help=--new-option should show the help from the comment in cli.rst
* qpdf --help=topic should list --new-option for the correct topic
* --new-option should appear in the manual
* --new-option should be in the command-line option index in the manual
* A Config method (in Config or one of the other Config classes in
  QPDFJob) should exist that corresponds to the command-line flag
* The job JSON file should have a new key in the schema corresponding
  to the new option

## RELEASE PREPARATION

* Each year, update copyright notices. This will find all relevant
  places (assuming current copyright is from last year):

  ```
  git --no-pager grep -i -n -P "copyright.*$(expr $(date +%Y) - 1).*berkenbilt"
  ```

  Also update the copyright in these places:
  * debian package -- search for copyright.*berkenbilt in debian/copyright
  * qtest-driver, TestDriver.pm in qtest source

  Copyright last updated: 2023.

* Take a look at "External Libraries" in TODO to see if we need to
  make any changes. There is still some automation work left to do, so
  handling external-libs releases is still manual. See also
  README-maintainer in external-libs.

* Check for open fuzz crashes at https://oss-fuzz.com

* Check all open issues and pull requests in github and the
  sourceforge trackers. Don't forget pull
  requests. Note: If the location for reporting issues changes, do a
  careful check of documentation and code to make sure any comments
  that include the issue creation URL are updated.

* Check `TODO` file to make sure all planned items for the release are
  done or retargeted.

* Check work `qpdf` project for private issues

* Make sure the code is formatted.

  ```
  ./format-code
  ```

* Run a spelling checker over the source code to catch errors in
  variable names, strings, and comments.

  ```
  ./spell-check
  ```

  This uses cspell. Install with `npm install -g cspell`. The output
  of cspell is suitable for use with `M-x grep` in emacs. Add
  exceptions to cSpell.json.

* If needed, run large file and image comparison tests by setting
  these environment variables:

  ```
  QPDF_LARGE_FILE_TEST_PATH=/full/path
  QPDF_TEST_COMPARE_IMAGES=1
  ```

  For Windows, use a Windows style path, not an MSYS path for large files.

* If any interfaces were added or changed, check C API to see whether
  changes are appropriate there as well. If necessary, review the
  casting policy in the manual, and ensure that integer types are
  properly handled with QIntC or the appropriate cast. Remember to
  ensure that any exceptions thrown by the library are caught and
  converted. See `trap_errors` in qpdf-c.cc.

* Double check versions and shared library details. They should
  already be up to date in the code.

  * Make sure version numbers are consistent in the following locations:
    * CMakeLists.txt
    * include/qpdf/DLL.h
    * manual/conf.py

  `make_dist` verifies this consistency, and CI fails if they are
  inconsistent.

* Update release notes in manual. Look at diffs and ChangeLog.
  Update release date in `manual/release-notes.rst`. Change "not yet
  released" to an actual date for the release.

* Add a release entry to ChangeLog: "x.y.z: release"

* Commit changes with title "Prepare x.y.z release"

* Performance test is included with binary compatibility steps. Even
  if releasing a new major release and not doing binary compatibility
  testing, do performance testing.

* Test for performance and binary compatibility:

  ```
  ./abi-perf-test v<old> @
  ```

  * Prefix with `SKIP_PERF=1` to skip performance test.
  * Prefix with `SKIP_TESTS=1` to skip test suite run.

  See "ABI checks" for details about the process.
  End state:
  * /tmp/check-abi/perf contains the performance comparison
  * /tmp/check-abi/old contains old sizes and library
  * /tmp/check-abi/new contains new sizes and library
  * run check_abi manually to compare

## CREATING A RELEASE

* Push to main. This will create an artifact called distribution
  which will contain all the distribution files. Download these,
  verify the checksums from the job output, rename to remove -ci from
  the names, and extract to the release archive area.

* Sign the source distribution:

```
version=x.y.z
gpg --detach-sign --armor qpdf-$version.tar.gz
```

* Build and test the debian package. This includes running autopkgtest.

* Add a calendar reminder to check the status of the debian package to
  make sure it is transitioning properly and to resolve any issues.

* From the release archive area, sign the releases.

```
\rm -f *.sha256
files=(*)
sha256sum ${files[*]} >| qpdf-$version.sha256
gpg --clearsign --armor qpdf-$version.sha256
mv qpdf-$version.sha256.asc qpdf-$version.sha256
chmod 444 *
chmod 555 *.AppImage
```

* When creating releases on github and sourceforge, remember to copy
  `README-what-to-download.md` separately onto the download area if
  needed.

* Ensure that the main branch has been pushed to github. The
  rev-parse command below should show the same commit hash for all its
  arguments. Create and push a signed tag. This should be run with
  HEAD pointing to the tip of main.

```
git rev-parse qpdf/main @
git tag -s v$version @ -m"qpdf $version"
git push qpdf v$version
```

* Update documentation branches

```
git push qpdf @:$(echo $version | sed -E 's/\.[^\.]+$//')
git push qpdf @:stable
```

* If this is an x.y.0 release, visit
  https://readthedocs.org/projects/qpdf/versions/ (log in with
  github), and activate the latest major/minor version

* Create a github release after pushing the tag. `gcurl` is an alias
  that includes the auth token.

```
# Create release

GITHUB_TOKEN=$(qdata-show cred github-token)
function gcurl() { curl -H "Authorization: token $GITHUB_TOKEN" ${1+"$@"}; }

url=$(gcurl -s -XPOST https://api.github.com/repos/qpdf/qpdf/releases -d'{"tag_name": "v'$version'", "name": "qpdf '$version'", "draft": true}' | jq -r '.url')

# Get upload url
upload_url=$(gcurl -s $url | jq -r '.upload_url' | sed -E -e 's/\{.*\}//')
echo $upload_url

# Upload all the files. You can add a label attribute too, which
# overrides the name.
for i in *; do
  mime=$(file -b --mime-type $i)
  gcurl -H "Content-Type: $mime" --data-binary @$i "$upload_url?name=$i"
done
```

If needed, go onto github and make any manual updates such as
indicating a pre-release, adding release notes, etc.

Template for release notes:

```
This is qpdf version x.y.z. (Brief description)

For a full list of changes from previous releases, please see the [release notes](https://qpdf.readthedocs.io/en/stable/release-notes.html). See also [README-what-to-download](./README-what-to-download.md) for details about
the available source and binary distributions.
```

```
# Publish release
gcurl -XPOST $url -d'{"draft": false}'
```

* Upload files to sourceforge.

```
rsync -vrlcO ./ jay_berkenbilt,qpdf@frs.sourceforge.net:/home/frs/project/q/qp/qpdf/qpdf/$version/
```

* On sourceforge, make the source package the default for all but
  Windows, and make the 64-bit msvc build the default for Windows.

* Publish a news item manually on sourceforge.

* Upload the debian package and Ubuntu ppa backports.

* Email the qpdf-announce list.

## RUNNING pikepdf's TEST SUITE

We run pikepdf's test suite from CI. These instructions show how to do
it manually.

Do this in a separate shell.

```
cd ...qpdf-source-tree...
export QPDF_SOURCE_TREE=$PWD
export QPDF_BUILD_LIBDIR=$QPDF_SOURCE_TREE/build/libqpdf
export LD_LIBRARY_PATH=$QPDF_BUILD_LIBDIR
rm -rf /tmp/z
mkdir /tmp/z
cd /tmp/z
git clone git@github.com:pikepdf/pikepdf
python3 -m venv v
source v/bin/activate
cd pikepdf
python3 -m pip install --upgrade pip
python3 -m pip install '.[test]'
rehash
python3 -m pip install .
pytest -n auto
```

If there are failures, use git bisect to figure out where the failure
was introduced. For example, set up a work area like this:

```
rm -rf /tmp/z
mkdir /tmp/z
cd /tmp/z
git clone file://$HOME/source/qpdf/qpdf/.git qpdf
git clone git@github.com:pikepdf/pikepdf
export QPDF_SOURCE_TREE=/tmp/z/qpdf
export QPDF_BUILD_LIBDIR=$QPDF_SOURCE_TREE/build/libqpdf
export LD_LIBRARY_PATH=$QPDF_BUILD_LIBDIR
cd qpdf
mkdir build
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
   -DMAINTAINER_MODE=1 -DBUILD_STATIC_LIBS=0 \
   -DCMAKE_BUILD_TYPE=RelWithDebInfo
cat <<'EOF'
#!/bin/bash
cd /tmp/z/pikepdf
cmake --build /tmp/z/qpdf/build -j16 --target libqpdf -- -k
git clean -dfx
rm -rf ../v
python3 -m venv ../v
source ../v/bin/activate
python3 -m pip install --upgrade pip
python3 -m pip install '.[test]'
python3 -m pip install .
pytest -n auto
EOF
chmod +x /tmp/check
```

Then in /tmp/z/qpdf, run git bisect. Use /tmp/check at each stage to
test whether it's a good or bad commit.

## OTHER NOTES

For local iteration on the AppImage generation, it works to just
./build-scripts/build-appimage and get the resulting AppImage from the
distribution directory. You can pass additional arguments to
build-appimage, which passes them along to to docker.

Use -e SKIP_TESTS=1 to skip the test suite.
Use -ti -e RUN_SHELL=1 to run a shell instead of the build script.

To iterate on the scripts directly in the source tree, you can run

```
docker build -t qpdfbuild appimage
docker run --privileged --rm -ti -e SKIP_TESTS=1 -e RUN_SHELL=1 \
       -v $PWD/..:/tmp/build ${1+"$@"} qpdfbuild
```

This will put you at a shell prompt inside the container with your
current directory set to the top of the source tree and your uid equal
to the owner of the parent directory source tree.

Note: this will leave some extra files (like .bash_history) in the
parent directory of the source tree. You will want to clean those up.

## DEPRECATION

This is a reminder of how to use and test deprecation.

To temporarily disable deprecation warnings for testing:

```cpp
#ifdef _MSC_VER
# pragma warning(disable : 4996)
#endif
#if (defined(__GNUC__) || defined(__clang__))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    // Do deprecated thing here
#if (defined(__GNUC__) || defined(__clang__))
# pragma GCC diagnostic pop
#endif
```

To declare something as deprecated:

```cpp
[[deprecated("explanation")]]
```

## LOCAL WINDOWS TESTING PROCEDURE

This is what I do for routine testing on Windows.

* From Windows, git clone from my Linux clone, and unzip
  `external-libs`.

* Start a command-line shell for x86_64 and x86 tools from Visual
  studio. From there, start C:\msys64\mingw64 twice and
  C:\msys64\mingw32 twice.

* Create a build directory for each of the four permutations. Then, in
  each build directory, run `../cmake-win <tool> maint`.

* Run `cmake --build . -j4`. For MSVC, add `--config Release`

* Test with with msvc: `ctest --verbose -C Release`

* Test with mingw:  `ctest --verbose -C RelWithDebInfo`

## DOCS ON readthedocs.org

* Registered for an account at readthedocs.org with my github account
* Project page: https://readthedocs.org/projects/qpdf/
* Docs: https://qpdf.readthedocs.io/
* Admin -> Settings
  * Set project home page
  * Advanced
    * Show version warning
    * Default version: stable
  * Email Notifications: set email address for build failures

At this time, there is nothing in .github/workflows to support this.
It's all set up as an integration directly between github and
readthedocs.

The way readthedocs.org does stable and versions doesn't exactly work
for qpdf. My tagging convention is different from what they expect,
and I don't need versions for every point release. I have the
following branching strategy to support docs:

* x.y -- points to the latest x.y.z release
* stable -- points to the latest release

The release process includes updating the approach branches and
activating versions.

## CMAKE notes

To verify the various cmake options and their interactions, several
manual tests were done:

* Break installed qpdf executables (qpdf, fix-qdf, zlib-flate), the
  shared library, and DLL.h to ensure that other qpdf installations do
  not interfere with building from source

* Build static only and shared only

* Install separate components separately

* Build only HTML docs and only PDF docs

* Try MAINTAINER_MODE without BUILD_DOC

We are using RelWithDebInfo for mingw and other non-Windows builds but
Release for MSVC. There are linker warnings if MSVC is built with
RelWithDebInfo when using external-libs.

## ABI checks

Until the conversion of the build to cmake, we relied on running the
test suite with old executables and a new library. When QPDFJob was
introduced, this method got much less reliable since a lot of public
API doesn't cross the shared library boundary. Also, when switching to
cmake, we wanted a stronger check that the library had the expected
ABI.

Our ABI check now consists of three parts:

* The same check as before: run the test suite with old executables
  and a new library

* Do a literal comparison of the symbols in the old and new shared
  libraries -- this is a strong test of ABI change

* Do a check to ensure that object sizes didn't change -- even with no
  changes to the API of exported functions, size changes break API

The combination of these checks is pretty strong, though there are
still things that could potentially break ABI, such as

* Changes to the types of public or protected data members without
  changing the size

* Changes to the meanings of parameters with changing the signature

Not breaking ABI/API still requires care.

The check_abi script is responsible for performing many of these
steps. See comments in check_abi for additional notes. Running
"check_abi check-sizes" is run by ctest on Linux when CHECK_SIZES is
on.

## CODE FORMATTING

* Emacs doesn't indent breaking strings concatenated with + over
  lines but clang-format does. It's clearer with clang-format. To
  get emacs and clang-format to agree, parenthesize the expression
  that builds the concatenated string.

* With

  ```cpp
  long_function(long_function(
      args)

  ```

  clang-format anchors relative to the first function, and emacs
  anchors relative to the second function. Use

  ```cpp
  long_function(
      // line-break
      long_function(
	  args)
  ```
  to resolve.

In the revision control history, there is a commit around April 3,
2022 with the title "Update some code manually to get better
formatting results" that shows several examples of changing code so
that clang-format produces several results. (In git this is commit
77e889495f7c513ba8677df5fe662f08053709eb.)

The commits that have the bulk of automatic or mechanical reformatting are
listed in .git-blame-ignore-revs. Any new bulk updates should be added there.

[//]: # (cSpell:ignore pikepdfs readthedocsorg .)
