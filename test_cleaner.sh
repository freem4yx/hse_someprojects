#!/bin/bash

# Тестовый скрипт для log_cleaner.sh
# Патык Дмитрий, Шошинов Николай 24КНТ2

MAIN_SCRIPT="./log_cleaner.sh"
TEST_LOG_DIR="./test_log"
TEST_BACKUP_DIR="./test_backup"
TOTAL_TESTS=0
PASSED_TESTS=0
flag_of_testing="$1"

#Func for old logs
create_test_files() {
    local dir="$1"
    local file_count="$2"
    local file_size_mb="$3"

    echo "Creating $file_count test files of $file_size_mb MB each in $dir"
    mkdir -p "$dir"

    for i in $(seq 1 "$file_count"); do
        filename="$dir/testfile_$(date +%Y%m%d_%H%M%S)_${i}.log"

        #Create
        dd if=/dev/urandom of="$filename" bs=1M count="$file_size_mb" status=none

        #Time
        touch -d "$i hours ago" "$filename"
    done

    #TOtal
    total_size=$(du -sh "$dir" | cut -f1)
    echo "Total test data size: $total_size"
}

#Func for cleaning
cleanup_test() {
    echo "Cleaning test directories..."
    rm -rf "$TEST_LOG_DIR"/* "$TEST_BACKUP_DIR"
}

mounting() {
    dd if=/dev/zero of=test.img bs=1M count=1024 status=progress
    mkfs.ext4 test.img
    mkdir "$TEST_LOG_DIR"
    fuse2fs -o fakeroot test.img "$TEST_LOG_DIR"
}

unmounting() {
    fusermount -u "$TEST_LOG_DIR"
    rmdir "$TEST_LOG_DIR"
    rm test.img
}

#Func for running
run_test() {
    local test_name="$1"
    local threshold="$2"
    local expected_result="$3"

    echo ""
    echo "=== $test_name ==="
    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    #Script
    "$MAIN_SCRIPT" "$TEST_LOG_DIR" "$TEST_BACKUP_DIR" "$threshold" "$files_to_archive"
    local result=$?

    #Result
    if [ "$result" -eq "$expected_result" ]; then
        echo "✓ $test_name PASSED"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo "✗ $test_name FAILED (Expected: $expected_result, Got: $result)"
        return 1
    fi
}

#Cheking

if [ "$flag_of_testing" == "--clear" ]; then
    cleanup_test
    unmounting
    exit 0
fi

if [ ! -f "$MAIN_SCRIPT" ]; then
    echo "Error: Main script $MAIN_SCRIPT not found!"
    echo "Make sure both scripts are in the same directory"
    exit 1
fi

echo "Starting tests for log_cleaner.sh"
echo "=================================="

echo "Mounting virtual drive:"
mounting
echo "Mounting completed!"

if [ "$flag_of_testing" == "--test1" ]; then
    echo "Preparing Test 1..."
    cleanup_test
    create_test_files "$TEST_LOG_DIR" 6 100
    run_test "Test 1: Normal operation" 1 0
    exit 0
fi

if [ "$flag_of_testing" == "--test2" ]; then
    echo "Preparing Test 2..."
    cleanup_test
    create_test_files "$TEST_LOG_DIR" 3 10
    run_test "Test 2: Below threshold" 90 0
    exit 0
fi

if [ "$flag_of_testing" == "--test3" ]; then
    echo "Preparing Test 3..."
    cleanup_test
    create_test_files "$TEST_LOG_DIR" 4 50
    run_test "Test 3: Archive all files" 1 0
    exit 0
fi

if [ "$flag_of_testing" == "--test4" ]; then
    echo "Preparing Test 4..."
    cleanup_test
    run_test "Test 4: Invalid directory" 1 1
    exit 0
fi

if [ "$flag_of_testing" == "--test5" ]; then
    echo "Preparing Test 5..."
    cleanup_test
    create_test_files "$TEST_LOG_DIR" 120 5
    run_test "Test 5: Many small files" 40 0
    exit 0
fi

if [ "$flag_of_testing" == "--test6" ]; then
    cleanup_test
    create_test_files "$TEST_LOG_DIR" 80 5
    LAB1_MAX_COMPRESSION=1 run_test "Test 6: Many small files via lzma" 40 0
    exit 0
fi

#Test 1 Normal
echo "Preparing Test 1..."
cleanup_test
create_test_files "$TEST_LOG_DIR" 6 100
run_test "Test 1: Normal operation" 1 0

#Test 2 Limit do not go through
echo "Preparing Test 2..."
cleanup_test
create_test_files "$TEST_LOG_DIR" 3 10
run_test "Test 2: Below threshold" 90 0

#Test 3 Archive all
echo "Preparing Test 3..."
cleanup_test
create_test_files "$TEST_LOG_DIR" 4 50
run_test "Test 3: Archive all files" 1 0

#Test 4 Fantom dir
echo "Preparing Test 4..."
cleanup_test
run_test "Test 4: Invalid directory" 1 1

#Test 5 many small
echo "Preparing Test 5..."
cleanup_test
create_test_files "$TEST_LOG_DIR" 120 5
run_test "Test 5: Many small files" 40 0

#Test 6 many small via lzma
echo "Preparing Test 6..."
cleanup_test
create_test_files "$TEST_LOG_DIR" 80 5
LAB1_MAX_COMPRESSION=1 run_test "Test 6: Many small files via lzma" 40 0

#Final res
echo ""
echo "--------------------------"
echo "Test Results: $PASSED_TESTS/$TOTAL_TESTS passed"

#Clean
cleanup_test
unmounting

if [ "$PASSED_TESTS" -eq "$TOTAL_TESTS" ]; then
    echo "✓ All tests passed!"
    exit 0
else
    echo "✗ Some tests failed!"
    exit 1
fi
