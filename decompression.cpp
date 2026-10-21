
// ---------------------- DECOMPRESS ----------------------
void decompressFile()
{
    ifstream input("compressed.bin", ios::binary);
    if (!input)
    {
        cout << "\ncompressed.bin not found!\n";
        return;
    }

    // Step 0: read the mode byte (0 = stored as is, 1 = Huffman coded)
    char mode;
    if (!input.get(mode) || (mode != 0 && mode != 1))
    {
        cout << "\nInvalid compressed file!\n";
        return;
    }

    if (mode == 0)
    {
        ofstream raw("decompressed.txt", ios::binary);
        char c;
        while (input.get(c))
            raw.put(c);
        cout << "\nDecompression completed: decompressed.txt\n";
        return;
    }

    // Step 1: read the header and rebuild the frequency table
    int freq[256] = {0};
    char u;
    input.get(u);
    int unique = (unsigned char)u + 1;      // we stored unique - 1

    for (int i = 0; i < unique; i++)
    {
        char c;
        int count;
        input.get(c);
        input.read((char*)&count, sizeof(int));
        freq[(unsigned char)c] = count;
    }

    // Step 2: build the SAME tree again
    Node *root = buildTree(freq);

    long total = 0;                  // total characters in original file
    for (int i = 0; i < 256; i++)
        total += freq[i];

    ofstream output("decompressed.txt", ios::binary);

    // special case: only one unique character
    if (isLeaf(root))
    {
        for (long i = 0; i < total; i++)
            output.put((char)root->ch);
    }
    else
    {
        // Step 3: read bits and walk the tree
        Node *current = root;
        long count = 0;
        char byte;

        while (input.get(byte) && count < total)
        {
            for (int i = 7; i >= 0 && count < total; i--)
            {
                int bit = ((unsigned char)byte >> i) & 1;

                if (bit == 0)
                    current = current->left;
                else
                    current = current->right;

                if (isLeaf(current))     // reached a character
                {
                    output.put((char)current->ch);
                    count++;
                    current = root;      // start again from the top
                }
            }
        }
    }

    input.close();
    output.close();
    deleteTree(root);

    cout << "\nDecompression completed: decompressed.txt\n";
}

