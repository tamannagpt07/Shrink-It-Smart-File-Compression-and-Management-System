void detectDuplicate(char file1[], char file2[])
{
    ifstream first(file1, ios::binary);
    ifstream second(file2, ios::binary);

    if (!first || !second)
    {
        cout << "\nCould not open one or both files.\n";
        return;
    }

    if (getFileSize(file1) != getFileSize(file2))
    {
        cout << "\nNOT duplicates (different sizes).\n";
        return;
    }

    char a, b;
    int same = 1;

    while (first.get(a))
    {
        second.get(b);
        if (a != b)
        {
            same = 0;
            break;
        }
    }

    if (same)
        cout << "\nDUPLICATE DETECTED! Both files have identical content.\n";
    else
        cout << "\nNOT duplicates. Contents are different.\n";
}
