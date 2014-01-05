//
//  WuManber.h
//  MCD
//
// Implementation of Wu Manber's Multi-Pattern Search Algorithm
// Implemented by Ray Burkholder, ray@oneunified.net
// Copyright (2008) One Unified
// For use without restriction but one:  this copyright notice must be preserved.

#ifndef __WUMANBER_H
#define __WUMANBER_H
#include <vector>
#include <string>
using namespace std;

typedef const vector<string> WuVector;


class WuManber {
public:
    WuManber (void);
    virtual ~WuManber( void );
    void Initialize( WuVector &patterns, 
                    bool bCaseSensitive = false, bool bIncludeSpecialCharacters = false, bool bIncludeExtendedAscii = false );
    string Search( size_t TextLength, const char *Text, std::vector<std::string> &patterns, int &start_pos );
    
protected:
    size_t k;  // number of patterns;
    size_t m;  // largest common pattern length
    
    size_t k_saved;
    size_t m_saved;
    
    size_t B;  // Wu Manber paper suggests B is 2 or 3 
    // small number of patterns, use B=2, use an exact table
    // for large number of patterns, use B=3 use compressed table (their code uses 400 as a cross over )
    static unsigned char rchExtendedAscii[];
    static char rchSpecialCharacters[];
    
    bool m_bInitialized;
    
    struct structAlphabet {
        char letter;  // letter for matching purposes
        unsigned char offset; // index of character in offsetted alphabet for shift and hash tables
    } m_lu[256]; // defines our alphabet for matching purposes, is LookUp table of letters for pattern/text matching
    
    unsigned char m_nSizeOfAlphabet;
    unsigned short m_nBitsInShift; // used for hashing blocks of B characters
    size_t m_nTableSize;  // size for SHIFT and HASH tables
    size_t * m_ShiftTable;  // SHIFT table
    
    struct structPatternMap { // one struct per pattern for this hash
        size_t PrefixHash;  // hash of first two characters of the pattern
        size_t ix;  // index into patterns for final comparison
    } m_PatternMapElement;  // termporary area for element storage
    
    vector<structPatternMap> *m_vPatternMap;
    // this is a combination of HASH and PREFIX table
    // the paper suggests shifting hash right by n bits to hash into this table in order to reduce sparseness
    
    
private:
};

#endif
