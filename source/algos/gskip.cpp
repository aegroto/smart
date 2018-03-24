/*
 * SMART: string matching algorithms research tool.
 * Copyright (C) 2012  Simone Faro and Thierry Lecroq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 * contact the authors at: faro@dmi.unict.it, thierry.lecroq@univ-rouen.fr
 * download the tool at: http://www.dmi.unict.it/~faro/smart/
 *
 * This is an alternative implementation of the Alpha Skip Search algorithm
 * in C. Charras and T. Lecroq and J. D. Pehoushek. 
 * A Very Fast String Matching Algorithm for Small Alphabets and Long Patterns. 
 * Proceedings of the 9th Annual Symposium on Combinatorial Pattern Matching, 
 * Lecture Notes in Computer Science, n.1448, pp.55--64, Springer-Verlag, Berlin, rutgers, (1998).
 * 
 * Implementation and slight modifications to the algorithm by Lorenzo Catania.
 * This file is totally no-standard and is made only for testing purposes.
 * Take a look at the original repository for a more versatile and usable implementation: 
 * https://github.com/aegroto/CLP-StringMatching-CPP
 */ 

using namespace std;

#include <string>
#include <cstring>
#include <cstddef>

#include "./include/main.h"

int log(int a, int base) {
    int result = 1, tmp = a;

    while(tmp > base) {
        tmp /= base;
        ++result;    
    }

    return result;
}

/** GAMMA NODE **/

class GammaNode {
    public:
        class Position {
            public:
                const int pos;
                Position* nextPos;

                Position(int);
                ~Position();
        };
        
    private:     
        GammaNode **children;
        Position *firstPos;

        const size_t arraySize, arrayOffset;
    public:
        GammaNode(char, char);
        ~GammaNode();

        inline GammaNode* get(int c) { if(c >= arrayOffset) return children[c]; return NULL; } 
        Position* getFirstPos() { return firstPos; }

        inline void set(char c, GammaNode *node) { children[c] = node; }  
        void addPos(int);  
};

GammaNode::GammaNode(char minChar, char maxChar) : arraySize(maxChar - minChar + 1), arrayOffset(minChar) {
    children = new GammaNode*[arraySize];
    children -= arrayOffset;

    const size_t limit = arrayOffset + arraySize;

    for(int i = arrayOffset; i < limit; ++i) {
        children[i] = NULL;
    }

    firstPos = NULL;
}

GammaNode::~GammaNode() {
    for(int i = arrayOffset; i < arraySize; ++i) {
        if(children[i] != NULL)
            delete children[i];
    }

    children += arrayOffset;
    delete[] children;
    delete firstPos;
}

GammaNode::Position::Position(int _pos) : pos(_pos) {
    nextPos = NULL;
}

GammaNode::Position::~Position() {
    if(nextPos != NULL)
        delete nextPos;
}

void GammaNode::addPos(int pos) {
    if(firstPos != NULL) {
        Position* lastPos = firstPos;
        while(lastPos->nextPos != NULL)
            lastPos = lastPos->nextPos;
        
        lastPos->nextPos = new Position(pos);
    } else {
        firstPos = new Position(pos);
    }
}

/** GAMMA TRIE **/

class GammaTrie {
    private:
        GammaNode *root;
        size_t l;
        char minChar, maxChar;

    public:
        GammaTrie(const char*, size_t, size_t, char, char);
        ~GammaTrie();
        
        GammaNode* getRoot() { return root; }
};

GammaTrie::GammaTrie(const char* str, size_t m, size_t _l, char _minChar, char _maxChar) {
    l = _l;
    minChar = _minChar;
    maxChar = _maxChar;

    root = new GammaNode(minChar, maxChar);

    GammaNode *node, *child;
    size_t foundChars;
    const size_t limit = m - l + 1;
    char* sub;

    for(size_t k = 0; k < limit; ++k) {
        node = root;
        child = NULL;
        foundChars = 0;
        sub = (char*) (str + k);

        child = node->get(sub[foundChars]);

        while(foundChars < l && child != NULL) {
            ++foundChars;
            
            
            node = child;        
            child = node->get(sub[foundChars]);        
        }

        while(foundChars < l) {
            child = new GammaNode(minChar, maxChar);
            node->set(sub[foundChars], child);
            node = child;

            ++foundChars;
        }

        if(child == NULL)
            node->addPos(k);
        else
            child->addPos(k);
    }
}

GammaTrie::~GammaTrie() {
    delete root;
}


/** MATCHER **/

class GammaSkipSearchMatcher {
    private:
        const char *x, *y;
        size_t m, n, sigma;
        char minChar, maxChar;
        int l, occurrences;

        bool preprocessed, searched;

        GammaTrie *trie;

        bool attempt(int);

        void report(int);
    public:
        GammaSkipSearchMatcher(string&, string&, char, char);
        GammaSkipSearchMatcher(const char*, size_t, const char*, size_t, char, char);
        ~GammaSkipSearchMatcher();
        
        void preprocessing();
        void search();
        
        void execute();
        void printOutput();

        int getOccurrences() { return occurrences; }
};

GammaSkipSearchMatcher::GammaSkipSearchMatcher(string& sx, string& sy, char _minChar, char _maxChar) {
    x = sx.c_str();
    y = sy.c_str();
    
    m = sx.length();
    n = sy.length();
    
    GammaSkipSearchMatcher(x, m, y, n, _minChar, _maxChar);
}

GammaSkipSearchMatcher::GammaSkipSearchMatcher(const char* x, size_t m, const char* y, size_t n, char _minChar, char _maxChar) :
    x(x), m(m), y(y), n(n) {
    minChar = _minChar;
    maxChar = _maxChar;

    sigma = maxChar - minChar + 1;

    occurrences = 0;

    preprocessed = searched = false;
}

GammaSkipSearchMatcher::~GammaSkipSearchMatcher() {
    delete trie;
}

void GammaSkipSearchMatcher::preprocessing() {
    if(preprocessed) return;
    
    l = log(m, sigma);

    trie = new GammaTrie(x, m, l, minChar, maxChar);

    preprocessed = true;
}

bool GammaSkipSearchMatcher::attempt(int start) {
    int c = -1;
    char *ty = (char*) (y + start);

    while(++c < m)
        if(x[c] != ty[c])
            break;

    return c == m;
}

void GammaSkipSearchMatcher::search() {
    if(searched) return;

    int j, k, start;
    GammaNode* node;
    
    j = m - l;
    
    const int limit = n - l + 1, 
              shift = m - l + 1;

    char* ty;
    while(j < limit) {
        node = trie->getRoot();
        ty = (char*) (y + j);

        for(k = 0; k < l && node != NULL; ++k) {
            node = node->get(ty[k]);
        }

        if(node != NULL) {
            GammaNode::Position* position = node->getFirstPos();

            while(position != NULL) {
                start =  j - position->pos;

                if(attempt(start)) {
                    report(start);
                }

                position = position->nextPos;
            }
        } 

        j += shift;
    }

    searched = true;
}

void GammaSkipSearchMatcher::report(int index) {
    ++occurrences;
}

void GammaSkipSearchMatcher::execute() {
    if(preprocessed && searched) return;

    BEGIN_PREPROCESSING
    preprocessing();
    END_PREPROCESSING

    BEGIN_SEARCHING
    search();
    END_SEARCHING
}

int search(unsigned char *x, int m, unsigned char *y, int n) {
    GammaSkipSearchMatcher matcher((const char*) x, m, (const char*) y, n, 0, 1);

    matcher.execute();

    return matcher.getOccurrences();
}