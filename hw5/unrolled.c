//Loop Unroll helper
inline void unroll_helper(const unsigned int i, const unsigned int LDA, const unsigned int ncols,
                          const unsigned int nrows, char* outboard, char* inboard)
{
    // Optimization Note: Simplify mod calculation
    const int inorth = i ? (i-1) : nrows-1;
    const int isouth = (i != nrows-1) ? i+1 : 0;
    
    //LDA setup needed to replace BOARD macro
    const unsigned int LDA_inorth = LDA * inorth;
    const unsigned int LDA_isouth = LDA * isouth;
    const unsigned int LDA_i = LDA*i;

    /* As we move across the board, there is overlap when counting neighbours:
     * Neighbour of X:
     * OAA
     * OXY
     * OAA
     *
     * Neighbours of Y:
     * AAO
     * XYO
     * AAO
     *
     * Notice neighbours, "A", overlap. We can reduce the computation by storing their sum
     */
    char prev = 
        inboard[LDA_inorth + ncols-1] +
        inboard[LDA_isouth + ncols-1];
    char cur = 
        inboard[LDA_inorth] +
        inboard[LDA_isouth]; 

    unsigned int j;
    for(j=0; j<ncols; j++) 
    {
        const int jwest = j ? j-1 : ncols-1;
        const int jeast = (j != ncols-1) ? j+1 : 0;

        const char next = 
            inboard[LDA_inorth + jeast] +
            inboard[LDA_isouth + jeast];

        // Optimization Note : Removed extra pointer access + LICM
        const char neighbor_count =
            cur +
            prev +
            next +
            inboard[LDA_i + jwest] +
            inboard[LDA_i+jeast];

        prev = cur;
        cur = next;
            
        // Optimization Note : Replace alivep algorithm
        unsigned char alivep_result = 0;
        if(neighbor_count == 3)
                alivep_result = 1;
        else if (neighbor_count == 2 && inboard[LDA_i+j])
                alivep_result = 1;
        
        outboard[LDA_i+j] = alivep_result;
    }
}
 

