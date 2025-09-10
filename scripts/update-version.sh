#!/bin/bash

# update-version.sh - Update version numbers in code
# Usage: ./update-version.sh <new_version>
# Example: ./update-version.sh 0.1.2

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <new_version>"
    echo "Example: $0 0.1.2"
    exit 1
fi

NEW_VERSION="$1"

# Validate version format (semantic versioning)
if ! [[ $NEW_VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Error: Version must be in format X.Y.Z (semantic versioning)"
    exit 1
fi

# Split version into major, minor, patch
IFS='.' read -r MAJOR MINOR PATCH <<< "$NEW_VERSION"

echo "Updating version to $NEW_VERSION..."

# Update header file comment
sed -i "s/\* Version: [0-9]\+\.[0-9]\+\.[0-9]\+/* Version: $NEW_VERSION/" include/argparse/argparse.hpp

# Update version macros
sed -i "s/#define ARGPARSE_VERSION_MAJOR [0-9]\+/#define ARGPARSE_VERSION_MAJOR $MAJOR/" include/argparse/argparse.hpp
sed -i "s/#define ARGPARSE_VERSION_MINOR [0-9]\+/#define ARGPARSE_VERSION_MINOR $MINOR/" include/argparse/argparse.hpp
sed -i "s/#define ARGPARSE_VERSION_PATCH [0-9]\+/#define ARGPARSE_VERSION_PATCH $PATCH/" include/argparse/argparse.hpp
sed -i "s/#define ARGPARSE_VERSION \"[0-9]\+\.[0-9]\+\.[0-9]\+\"/#define ARGPARSE_VERSION \"$NEW_VERSION\"/" include/argparse/argparse.hpp

# Update test expectations
sed -i "s/EXPECT_EQ(ARGPARSE_VERSION_MAJOR, [0-9]\+);/EXPECT_EQ(ARGPARSE_VERSION_MAJOR, $MAJOR);/" tests/unit/basic_structure_test.cpp
sed -i "s/EXPECT_EQ(ARGPARSE_VERSION_MINOR, [0-9]\+);/EXPECT_EQ(ARGPARSE_VERSION_MINOR, $MINOR);/" tests/unit/basic_structure_test.cpp
sed -i "s/EXPECT_EQ(ARGPARSE_VERSION_PATCH, [0-9]\+);/EXPECT_EQ(ARGPARSE_VERSION_PATCH, $PATCH);/" tests/unit/basic_structure_test.cpp
sed -i "s/EXPECT_STREQ(ARGPARSE_VERSION, \"[0-9]\+\.[0-9]\+\.[0-9]\+\");/EXPECT_STREQ(ARGPARSE_VERSION, \"$NEW_VERSION\");/" tests/unit/basic_structure_test.cpp

echo "Version updated successfully!"
echo ""
echo "Changes made:"
echo "- include/argparse/argparse.hpp: Updated version comment and macros"
echo "- tests/unit/basic_structure_test.cpp: Updated test expectations"
echo ""
echo "Next steps:"
echo "1. Review the changes: git diff"
echo "2. Run tests: cmake -B tests/build -S tests && cmake --build tests/build && ctest --test-dir tests/build"
echo "3. Commit changes: git add . && git commit -m \"Update version to v$NEW_VERSION\""