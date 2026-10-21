void checkFile(char original[])
{
    ifstream a(original, ios::binary);
    ifstream b("decompressed.txt", ios::binary);

    if (!a || !b)
    {
        cout << "\nOriginal or decompressed file not found.\n";
        return;
    }

    char x, y;
    int same = 1;

    while (1)
    {
        int gotA = (a.get(x)) ? 1 : 0;
        int gotB = (b.get(y)) ? 1 : 0;

        if (gotA == 0 && gotB == 0)
            break;
        if (gotA != gotB || x != y)
        {
            same = 0;
            break;
        }
    }

    if (same)
        cout << "\nSUCCESS: Both files are identical.\n";
    else
        cout << "\nERROR: Files are different.\n";
}

