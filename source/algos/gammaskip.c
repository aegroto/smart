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
 * This is a Gamma Skip Search algorithm implementation
 * Lorenzo Catania, 2018
 */

#include "include/define.h"
#include "include/main.h"
#include <limits.h>

#include <math.h>

#define FIXED_ALPHABET

#define ALPHABET_SIZE 2
#define ALPHABET_MIN_CHAR 0
#define ALPHABET_MAX_CHAR 1

/***********/
/** UTILS **/
/***********/

int logarithm(int a, int base) {
    if(base > 1) {
        int result = 1, tmp = a;

        while(tmp > base) {
            tmp /= base;
            ++result;   
        }

        return result;
    }
    return 1;
}

/**************/
/** POSITION **/
/**************/

typedef struct _Position Position;
struct _Position {
    int pos;
    Position *nextPos;
};

void Position_init(Position* position, int pos) {
    position->pos = pos;
    position->nextPos = NULL;
}

void Position_destroy(Position* position) {
    if(position->nextPos != NULL) {
        Position_destroy(position->nextPos);
        free(position->nextPos);
    }
}

/****************/
/** GAMMA NODE **/
/****************/

typedef struct _GammaNode GammaNode;
struct _GammaNode {
    GammaNode **children;
    Position *firstPos;
    size_t arraySize, arrayOffset;
};

void GammaNode_init(GammaNode* node, char minChar, char maxChar) {
    node->arraySize = maxChar - minChar + 1;
    node->arrayOffset = minChar;
    node->children = (GammaNode**) (malloc(sizeof(GammaNode*) * node->arraySize));
    node->children -= node->arrayOffset;

    const size_t limit = node->arrayOffset + node->arraySize;

    for(int i = node->arrayOffset; i < limit; ++i) {
        node->children[i] = NULL;
    }

    node->firstPos = NULL;
}

void GammaNode_destroy(GammaNode* node) {
    for(int i = node->arrayOffset; i < node->arraySize; ++i) {
        if(node->children[i] != NULL) {
            GammaNode_destroy(node->children[i]);
        }
    }

    node->children += node->arrayOffset;
    free(node->children);
    Position_destroy(node->firstPos);
    free(node->firstPos);
}

inline GammaNode* GammaNode_get(GammaNode* node, int c) {
    if(c >= node->arrayOffset)
        return node->children[c];

    return NULL;
}

void GammaNode_addPos(GammaNode* node, int pos) {
    if(node->firstPos != NULL) {
        Position* lastPos = node->firstPos;
        while(lastPos->nextPos != NULL)
            lastPos = lastPos->nextPos;
        
        lastPos->nextPos = (Position*) malloc(sizeof(Position));
        Position_init(lastPos->nextPos, pos);
    } else {
        node->firstPos = (Position*) malloc(sizeof(Position));
        Position_init(node->firstPos, pos);
    }
}

/****************/
/** GAMMA TRIE **/
/****************/

typedef struct _GammaTrie GammaTrie;

struct _GammaTrie {
        GammaNode *root;
        size_t l;
        char minChar, maxChar;
};

void GammaTrie_init(GammaTrie* trie, const char* str, size_t m, size_t l, char minChar, char maxChar) {
    trie->l = l;
    trie->minChar = minChar;
    trie->maxChar = maxChar;

    trie->root = (GammaNode*) malloc(sizeof(GammaNode));
    GammaNode_init(trie->root, minChar, maxChar);

    GammaNode *node, *child;
    size_t foundChars;
    const size_t limit = m - l + 1;
    char* sub;

    for(size_t k = 0; k < limit; ++k) {
        node = trie->root;
        child = NULL;
        foundChars = 0;
        sub = (char*) (str + k);

        child = GammaNode_get(node, sub[foundChars]);

        while(foundChars < l && child != NULL) {
            ++foundChars;
            
            node = child;        
            child = GammaNode_get(node, sub[foundChars]);        
        }

        while(foundChars < l) {
            child = (GammaNode*) malloc(sizeof(GammaNode));
            GammaNode_init(child, minChar, maxChar);

            node->children[sub[foundChars]] = child;
            node = child;

            ++foundChars;
        }

        if(child == NULL)
            GammaNode_addPos(node, k);
        else
            GammaNode_addPos(child, k);
    } 
}

void GammaTrie_destroy(GammaTrie* trie) {
    GammaNode_destroy(trie->root);
}

/**************************/
/** ALGORITHM PROCEDURES **/
/**************************/

typedef struct _GammaMatcher GammaMatcher;

struct _GammaMatcher {
    char *x, *y;
    size_t m, n, sigma;
    char minChar, maxChar;
    int l, occurrences;

    GammaTrie *trie;
};

void GammaMatcher_init(GammaMatcher* matcher, char* sx, char* sy, size_t m, size_t n, char minChar, char maxChar) {
    matcher->x = sx;
    matcher->y = sy;

    matcher->m = m;
    matcher->n = n;

    matcher->minChar = minChar; 
    matcher->maxChar = maxChar;

    matcher->sigma = maxChar - minChar + 1;

    matcher->occurrences = 0;
}

void GammaMatcher_preprocessing(GammaMatcher* matcher) {
    matcher->l = logarithm(matcher->m, matcher->sigma);

    matcher->trie = (GammaTrie*) malloc(sizeof(GammaTrie));
    GammaTrie_init(matcher->trie, matcher->x, matcher->m, matcher->l, matcher->minChar, matcher->maxChar);
}

void GammaMatcher_destroy(GammaMatcher* matcher) {
    GammaTrie_destroy(matcher->trie);
    free(matcher->trie);
}

int GammaMatcher_attempt(GammaMatcher* matcher, int start) {
    char *ty = (char*) (matcher->y + start);

    return strncmp(matcher->x, ty, matcher->m) == 0;
}

void GammaMatcher_search(GammaMatcher* matcher) {
    int j, k, start;
    GammaNode* node;
    
    j = matcher->m - matcher->l;
    
    const int limit = matcher->n - matcher->l + 1, 
              shift = matcher->m - matcher->l + 1;

    char* ty;
    while(j < limit) {
        node = matcher->trie->root;
        ty = (char*) (matcher->y + j);

        for(k = 0; k < matcher->l && node != NULL; ++k) {
            node = GammaNode_get(node, ty[k]);
        }

        if(node != NULL) {
            Position* position = node->firstPos;

            while(position != NULL) {
                start = j - position->pos;

                if(GammaMatcher_attempt(matcher, start)) {
                    matcher->occurrences++;
                }

                position = position->nextPos;
            }
        } 
        j += shift;
    }
}

/************/
/** SEARCH **/
/************/

int search(unsigned char *x, int m, unsigned char *y, int n) {
    BEGIN_PREPROCESSING
    GammaMatcher* matcher = (GammaMatcher*) malloc(sizeof(GammaMatcher)); 

#ifdef FIXED_ALPHABET
    GammaMatcher_init(matcher, x, y, m, n, ALPHABET_MIN_CHAR, ALPHABET_MAX_CHAR);
#else
    char minChar = CHAR_MAX, maxChar = CHAR_MIN;

    for(int i = 0; i < m; ++i) {
        if(x[i] < minChar) {
            minChar = x[i];
        } else if(x[i] > maxChar) {
            maxChar = x[i];
        }
    }

    for(int i = 0; i < n; ++i) {
        if(y[i] < minChar) {
            minChar = y[i];
        } else if(y[i] > maxChar) {
            maxChar = y[i];
        }
    }

    // printf("[gamma] minChar = %d, maxChar = %d\n", minChar, maxChar);
    GammaMatcher_init(matcher, x, y, m, n, minChar, maxChar);
#endif
    GammaMatcher_preprocessing(matcher);
    END_PREPROCESSING

    BEGIN_SEARCHING
    GammaMatcher_search(matcher);
    END_SEARCHING

    // printf("[gamma] exiting (%d)...\n", matcher->occurrences);

    int count = matcher->occurrences;
    free(matcher);
    return count;
}