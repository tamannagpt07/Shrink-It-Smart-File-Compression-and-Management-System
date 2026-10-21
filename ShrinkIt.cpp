#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
using namespace std;

// Each node of the Huffman tree
struct Node
{
    unsigned char ch;   // character (only meaningful in leaf nodes)
    int freq;           // how many times it appears
    Node *left;
    Node *right;
};

// Create a new node
Node* newNode(unsigned char c, int f)
{
    Node *n = new Node;
    n->ch = c;
    n->freq = f;
    n->left = NULL;
    n->right = NULL;
    return n;
}

// A leaf has no children
int isLeaf(Node *n)
{
    return (n->left == NULL && n->right == NULL);
}

// Free the memory of the whole tree
void deleteTree(Node *root)
{
    if (root == NULL)
        return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Find the position of the node with the smallest frequency in the list
int findMin(Node *list[], int n)
{
    int pos = 0;
    for (int i = 1; i < n; i++)
    {
        if (list[i]->freq < list[pos]->freq)
            pos = i;
    }
    return pos;
}

// Build the Huffman tree from the frequency table
Node* buildTree(int freq[])
{
    Node *list[256];
    int n = 0;

    // Step 1: make one leaf node for every character that appears
    for (int i = 0; i < 256; i++)
    {
        if (freq[i] > 0)
        {
            list[n] = newNode((unsigned char)i, freq[i]);
            n++;
        }
    }

    if (n == 0)
        return NULL;

    // Step 2: repeatedly join the two smallest nodes
    while (n > 1)
    {
        int a = findMin(list, n);
        Node *first = list[a];
        list[a] = list[n - 1];   // remove it by moving the last node here
        n--;

        int b = findMin(list, n);
        Node *second = list[b];
        list[b] = list[n - 1];
        n--;

        Node *parent = newNode(0, first->freq + second->freq);
        parent->left = first;
        parent->right = second;

        list[n] = parent;        // put the new parent back in the list
        n++;
    }

    return list[0];              // the last remaining node is the root
}

// Walk the tree and store the code of every character
// going left adds '0', going right adds '1'
void makeCodes(Node *root, char path[], int depth, char codes[][257])
{
    if (root == NULL)
        return;

    if (isLeaf(root))
    {
        if (depth == 0)          // only one unique character in the file
        {
            path[0] = '0';
            depth = 1;
        }
        path[depth] = '\0';
        strcpy(codes[root->ch], path);
        return;
    }

    path[depth] = '0';
    makeCodes(root->left, path, depth + 1, codes);

    path[depth] = '1';
    makeCodes(root->right, path, depth + 1, codes);
}

// Return the size of a file in bytes (-1 if it does not exist)
long getFileSize(const char name[])
{
    ifstream file(name, ios::binary);
    if (!file)
        return -1;
    file.seekg(0, ios::end);
    return (long)file.tellg();
}

// ---------------------- COMPRESS ----------------------
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

// ---------------------- CHECK INTEGRITY ----------------------
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

// ---------------------- FILE INFO ----------------------
void fileInfo(char filename[])
{
    long size = getFileSize(filename);
    if (size < 0)
    {
        cout << "\nFile not found!\n";
        return;
    }
    cout << "\nFile Name : " << filename << "\n";
    cout << "File Size : " << size << " bytes\n";
}

// ---------------------- DUPLICATE DETECTION ----------------------
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

// ---------------------- MAIN ----------------------
int main()
{
    int choice;
    char filename[100], file2[100];

    while (1)
    {
        cout << "\n==============================\n";
        cout << "           SHRINKIT\n";
        cout << "  Smart File Compression System\n";
        cout << "==============================\n";
        cout << "1. Compress File\n";
        cout << "2. Decompress File\n";
        cout << "3. Check File Integrity\n";
        cout << "4. File Information\n";
        cout << "5. Duplicate File Detection\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1)
        {
            cout << "Enter file name: ";
            cin >> filename;
            compressFile(filename);
        }
        else if (choice == 2)
        {
            decompressFile();
        }
        else if (choice == 3)
        {
            cout << "Enter original file name: ";
            cin >> filename;
            checkFile(filename);
        }
        else if (choice == 4)
        {
            cout << "Enter file name: ";
            cin >> filename;
            fileInfo(filename);
        }
        else if (choice == 5)
        {
            cout << "Enter first file name: ";
            cin >> filename;
            cout << "Enter second file name: ";
            cin >> file2;
            detectDuplicate(filename, file2);
        }
        else if (choice == 6)
        {
            cout << "\nThank you for using ShrinkIt!\n";
            break;
        }
        else
        {
            cout << "\nInvalid choice. Enter 1 to 6.\n";
        }
    }
    return 0;
}
