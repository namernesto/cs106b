#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "testing/SimpleTest.h"
#include "map.h"
using namespace std;

EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves, EncodingTreeNode* t);
Map<char, string> getTreeBits(EncodingTreeNode* tree, Map<char, string> currentMap, string sequence);
Map<char, int> getFrequencyMap(string text);

/**
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, decode the bits back to the original message text.
 *
 * You can assume that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 *
 * Your implementation may change the messageBits queue however you like. There
 * are no requirements about what it should look like after this function
 * returns. The encoding tree should be unchanged.
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    /* TODO: Implement this function. */
    string result = "";
    EncodingTreeNode* pointer = tree;

    while(!messageBits.isEmpty()){
        Bit bit = messageBits.dequeue();
        if(bit == 0){
            pointer = pointer->zero;
        } else{
            pointer = pointer->one;
        }
        if (pointer->isLeaf()){
            char s = pointer->getChar();
            result = result + s;
            pointer = tree;
        }
    }

    return result;
}

/**
 * Reconstruct encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * You can assume that the queues are well-formed and represent
 * a valid encoding tree.
 *
 * Your implementation may change the queue parameters however you like. There
 * are no requirements about what they should look like after this function
 * returns.
 *
 * The function takes treeshape and treeleaves as pass by reference and recursively
 * goes through the entire tree. It returns a full tree.
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    /* TODO: Implement this function. */
    Bit bit = treeShape.dequeue();
    if (bit == 0){
        return new EncodingTreeNode(treeLeaves.dequeue());
    } else{
        EncodingTreeNode* zero = unflattenTree(treeShape, treeLeaves);
        EncodingTreeNode* one = unflattenTree(treeShape, treeLeaves);
        return new EncodingTreeNode(zero, one);
    }
}


/**
 * Decompress the given EncodedData and return the original text.
 *
 * You can assume the input data is well-formed and was created by a correct
 * implementation of compress.
 *
 * Your implementation may change the data parameter however you like. There
 * are no requirements about what it should look like after this function
 * returns.
 *
 * This function first draws the tree by calling the unflattenTree and later decodes the text
 * by calling the decodetext function and taking the tree and data's messageBits as inputs.
 * It retuns a string, which is the data decompressed.
 */
string decompress(EncodedData& data) {
    /* TODO: Implement this function. */
    EncodingTreeNode* tree = unflattenTree(data.treeShape, data.treeLeaves);
    string result = decodeText(tree, data.messageBits);
    deallocateTree(tree);
    return result;
}

/**
 * Constructs an optimal Huffman coding tree for the given text, using
 * the algorithm described in lecture.
 *
 * Reports an error if the input text does not contain at least
 * two distinct characters.
 *
 * When assembling larger trees out of smaller ones, make sure to set the first
 * tree dequeued from the queue to be the zero subtree of the new tree and the
 * second tree as the one subtree.
 *
 * This function takes in a string and 1- builds a frequencymap by calling a helper function 2-
 * transform the frequencymap into a PriorityQueue and 3- iteratively builds a single parent
 * PriorityQueue to build the Huffman tree.
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    /* Get the frequencymap of the string */
    Map<char, int> frequencyMap = getFrequencyMap(text);

    // create a Priority queue. Loop through each key in map and enqueue the key to the pqueue as an node
    PriorityQueue<EncodingTreeNode*> pqueue;
    for(char c: frequencyMap.keys()){
        EncodingTreeNode* node = new EncodingTreeNode(c);
        pqueue.enqueue(node, frequencyMap[c]);
    }

    // while pqueue is not the single largest parent node, dequeue two child nodes, combine them, and enqueue it to the pqueue
    while(pqueue.size() > 1){
        // get the priority of the two child nodes for later when adding the sum of them as priority of parent node
        int child1_priority = pqueue.peekPriority();
        EncodingTreeNode* child1 = pqueue.dequeue();

        int child2_priority = pqueue.peekPriority();
        EncodingTreeNode* child2 = pqueue.dequeue();
        EncodingTreeNode* parent = new EncodingTreeNode(child1, child2);
        pqueue.enqueue(parent, child1_priority + child2_priority);
    }
    return pqueue.peek();
}

/* This is a helper function that takes in a string and returns a Map with the characters
 * of the string and their respective frequencies.
 */
Map<char, int> getFrequencyMap(string text){
    Map<char, int> frequencyMap;
    int letter_index = 0;
    for(char letter: text){
        if(!frequencyMap.containsKey(letter)){
            frequencyMap[letter] = 0;
            int counter = 1;
            for(int i = letter_index + 1; i < text.size(); i++){
                if(letter == text[i]){
                    counter++;
                }
            }
            frequencyMap[letter] = counter;
        }

        letter_index++;
    }
    return frequencyMap;
}

STUDENT_TEST("Test getFrequencyMaps") {
    string text = "hheelllo";
    Map<char, int> expect = {{'h', 2}, {'e', 2}, {'l', 3}, {'o', 1}};
    EXPECT_EQUAL(getFrequencyMap(text), expect);
    text = "BOOKKEEPER";
    expect = {{'B', 1}, {'O', 2}, {'K', 2}, {'E', 3}, {'P', 1}, {'R', 1}};
}

/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 *
 */
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {

    Map<char, string> currentMap;
    string sequence = "";
    Map<char, string> bitSequence = getTreeBits(tree, currentMap, sequence);
    Queue<Bit> result;
    for (char c: text){
        for (char b: bitSequence[c]){
            Bit bit = charToInteger(b);
            result.enqueue(bit);
        }
    }
    return result;
}

/* This is a helper function that goes through a tree in pre-order recursion and returns a
 * Map with the bitsquence of each character of a string.
 */
Map<char, string> getTreeBits(EncodingTreeNode* tree, Map<char, string> currentMap, string sequence){

    if(tree->isLeaf()){
        currentMap[tree->ch] = sequence;
    } else{
        currentMap += getTreeBits(tree->zero, currentMap, sequence + "0");
        currentMap += getTreeBits(tree->one, currentMap, sequence + "1");
    }
    return currentMap;
}

STUDENT_TEST("Test getTreeBits") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    Map<char, string> currentMap;
    string sequence = "";
    Map<char, string> expect = {{'T', "0"}, {'R', "100"}, {'S', "101"}, {'E', "11"}};
    Map<char, string> result = getTreeBits(tree, currentMap, sequence);
    EXPECT_EQUAL(expect, result);
    deallocateTree(tree);
}

STUDENT_TEST("Test getTreeBits") {

    EncodingTreeNode* tree = new EncodingTreeNode(new EncodingTreeNode('T'), new EncodingTreeNode('R'));
    Map<char, string> currentMap;
    string sequence = "";
    Map<char, string> expect = {{'T', "0"}, {'R', "1"}};
    Map<char, string> result = getTreeBits(tree, currentMap, sequence);
    EXPECT_EQUAL(expect, result);
    deallocateTree(tree);
}


/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a valid well-formed encoding tree.
 *
 * This function takes in a tree, treeShape, and treeLeaves as pass by reference.
 * It recursively creates the treeshape and treeleaves given the tree.
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {

    if (tree->isLeaf()){
        treeShape.enqueue(0);
        treeLeaves.enqueue(tree->getChar());
    } else{
        treeShape.enqueue(1);
        flattenTree(tree->zero, treeShape, treeLeaves);
        flattenTree(tree->one, treeShape, treeLeaves);
    }
}

/**
 * Compress the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 *
 */
EncodedData compress(string messageText) {
    /* Call buildHuffmanTree to create the tree given a string */
    EncodingTreeNode* tree = buildHuffmanTree(messageText);

    // Create a queue with valuetype of Bit that has the encoded text of the string
    Queue<Bit> messageBit = encodeText(tree, messageText);

    // initialize treeshape and treeleaves as empty. They are later created by calling flattenTree on them
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    flattenTree(tree, treeShape, treeLeaves);

    // Create an encoded data and fill data with its respective information
    EncodedData data;
    data.treeLeaves = treeLeaves;
    data.treeShape = treeShape;
    data.messageBits = messageBit;
    deallocateTree(tree);
    return data;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {
    /* Example encoding tree used in multiple test cases:
     *                *
     *              /   \
     *             T     *
     *                  / \
     *                 *   E
     *                / \
     *               R   S
     */
    EncodingTreeNode* t = new EncodingTreeNode('T');
    EncodingTreeNode* r = new EncodingTreeNode('R');
    EncodingTreeNode* s = new EncodingTreeNode('S');
    EncodingTreeNode* e = new EncodingTreeNode('E');
    EncodingTreeNode* rs = new EncodingTreeNode(r, s);
    EncodingTreeNode* rse = new EncodingTreeNode(rs, e);
    EncodingTreeNode* trse = new EncodingTreeNode(t, rse);
    return trse;
}

void deallocateTree(EncodingTreeNode* t) {
    if(t->zero == nullptr && t->one == nullptr){
        delete t;
    } else{
        deallocateTree(t->zero);
        deallocateTree(t->one);
        delete t;
    }
}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {

    if(a->isLeaf() && b->isLeaf()){
        if(a->getChar() == b->getChar()){
            return true;
        } else{
            return false;
        }
    } else if((!a->isLeaf() && b->isLeaf()) || (a->isLeaf() && !b->isLeaf())){
        return false;
    }
    if(areEqual(a->zero, b->zero) && areEqual(a->one, b->one)){
        return true;
    }

    return false;
}

/* * * * * * Test Cases Below This Point * * * * * */

STUDENT_TEST("Test deallocate") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    deallocateTree(tree);
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

PROVIDED_TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

PROVIDED_TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

PROVIDED_TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

PROVIDED_TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    encodeText(reference, "E");
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

PROVIDED_TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

PROVIDED_TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

PROVIDED_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman"
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
