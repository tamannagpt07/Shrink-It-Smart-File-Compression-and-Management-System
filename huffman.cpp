
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
