void compressFile(char filename[])
{
    ifstream input(filename, ios::binary);
    if (!input)
    {
        cout << "\nFile not found!\n";
        return;
    }

    // Step 1: count how many times each character appears
    int freq[256] = {0};
    char ch;
    long originalSize = 0;

    while (input.get(ch))
    {
        freq[(unsigned char)ch]++;
        originalSize++;
    }

    if (originalSize == 0)
    {
        cout << "\nFile is empty, nothing to compress.\n";
        return;
    }

    // Step 2: build the tree and the codes
    Node *root = buildTree(freq);
    char codes[256][257];
    char path[300];
    makeCodes(root, path, 0, codes);

    // Step 3: write the header (unique count + character/frequency pairs)
    int unique = 0;
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            unique++;

    ofstream output("compressed.bin", ios::binary);
    output.put((char)1);                    // mode byte: 1 = Huffman coded
    output.put((char)(unique - 1));         // 1 byte is enough (stores 0..255 for 1..256)

    for (int i = 0; i < 256; i++)
    {
        if (freq[i] > 0)
        {
            output.put((char)i);
            output.write((char*)&freq[i], sizeof(int));
        }
    }

    // Step 4: read the file again and write the codes bit by bit
    input.clear();
    input.seekg(0, ios::beg);

    unsigned char buffer = 0;   // collects 8 bits
    int bitCount = 0;

    while (input.get(ch))
    {
        char *code = codes[(unsigned char)ch];

        for (int i = 0; code[i] != '\0'; i++)
        {
            buffer = buffer << 1;
            if (code[i] == '1')
                buffer = buffer | 1;
            bitCount++;

            if (bitCount == 8)       // buffer is full, write one byte
            {
                output.put((char)buffer);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    // leftover bits: fill the remaining space with zeros and write
    if (bitCount > 0)
    {
        buffer = buffer << (8 - bitCount);
        output.put((char)buffer);
    }

    output.close();
    deleteTree(root);

    long compressedSize = getFileSize("compressed.bin");

    // Tiny files: the header can be bigger than the data we saved.
    // If Huffman made the file larger, store the original bytes as they are instead
    // (mode byte 0), so the output is at most 1 byte bigger than the input.
    int storedRaw = 0;
    if (compressedSize > originalSize + 1)
    {
        input.clear();
        input.seekg(0, ios::beg);

        ofstream raw("compressed.bin", ios::binary);
        raw.put((char)0);                   // mode byte: 0 = stored without compression
        while (input.get(ch))
            raw.put(ch);
        raw.close();

        compressedSize = getFileSize("compressed.bin");
        storedRaw = 1;
    }
    input.close();

    double saved = (double)(originalSize - compressedSize) / originalSize * 100;

    cout << "\n===== COMPRESSION COMPLETED =====\n";
    cout << "Original Size   : " << originalSize << " bytes\n";
    cout << "Compressed Size : " << compressedSize << " bytes\n";
    cout << "Space Saved     : " << saved << " %\n";
    cout << "Output file     : compressed.bin\n";
    if (storedRaw)
        cout << "Note: file is too small or too random for Huffman coding to help, so it was stored as is (+1 byte).\n";
    else if (compressedSize >= originalSize)
        cout << "Note: this file is too small/random to shrink (header adds extra bytes).\n";
}
