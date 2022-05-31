//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <math.h>

//
// TODO:Student Information
//
const char *studentName = "Feiyu Yang";
const char *studentID   = "A59009686";
const char *email       = "f8yang@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//
#define MAX_SETS 16384
#define MAX_ASSOS 16
//
//TODO: Add your Cache data structures here
uint32_t icache[MAX_SETS][MAX_ASSOS];
uint32_t dcache[MAX_SETS][MAX_ASSOS];
uint32_t l2cache[MAX_SETS][MAX_ASSOS];

uint32_t icacheLRU[MAX_SETS][MAX_ASSOS];
uint32_t dcacheLRU[MAX_SETS][MAX_ASSOS];
uint32_t l2cacheLRU[MAX_SETS][MAX_ASSOS];
//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
  for (int i=0; i<MAX_SETS; i++){
    for (int j=0; j<MAX_ASSOS; j++){
      icache[i][j] = 0;
      dcache[i][j] = 0;
      l2cache[i][j] = 0;

      icacheLRU[i][j] = j;
      dcacheLRU[i][j] = j;
      l2cacheLRU[i][j] = j;
    }
  }
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  icacheRefs++;
  uint32_t speed = 0;

  int byte_offset = log2(blocksize);
  int tag_offset = log2(blocksize*icacheSets);

  uint32_t set_index = (addr >> byte_offset) & (icacheSets-1);
  uint32_t tag = addr >> tag_offset;

  int flag =  0;

  for (int i=0; i<icacheAssoc; i++){
    if (icache[set_index][i] == tag){
      speed = icacheHitTime;
      flag = 1;

      for (int j=0; j<icacheAssoc; j++){
        if (icacheLRU[set_index][j] == i){
          for (int k=j; k<icacheAssoc-1; k++){
            icacheLRU[set_index][k] = icacheLRU[set_index][k+1];
          }
          icacheLRU[set_index][icacheAssoc-1] = i;
          break;
        }
      }

      break;
    }
  }

  if (flag == 0){
    speed = icacheHitTime + l2cache_access(addr);
    icacheMisses++;
    icachePenalties += speed;
    
    uint32_t currentLRU = icacheLRU[set_index][0];
    icacheLRU[set_index][currentLRU] = tag;
    for (int k=0; k<icacheAssoc-1; k++){
      icacheLRU[set_index][k] = icacheLRU[set_index][k+1];
    }
    icacheLRU[set_index][icacheAssoc-1] = currentLRU;
  }

  return speed;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  dcacheRefs++;
  uint32_t speed = 0;

  int byte_offset = log2(blocksize);
  int tag_offset = log2(blocksize*dcacheSets);

  uint32_t set_index = (addr >> byte_offset) & (dcacheSets-1);
  uint32_t tag = addr >> tag_offset;

  int flag =  0;

  for (int i=0; i<dcacheAssoc; i++){
    if (dcache[set_index][i] == tag){
      speed = dcacheHitTime;
      flag = 1;

      for (int j=0; j<dcacheAssoc; j++){
        if (dcacheLRU[set_index][j] == i){
          for (int k=j; k<dcacheAssoc-1; k++){
            dcacheLRU[set_index][k] = dcacheLRU[set_index][k+1];
          }
          dcacheLRU[set_index][dcacheAssoc-1] = i;
          break;
        }
      }

      break;
    }
  }

  if (flag == 0){
    speed = dcacheHitTime + l2cache_access(addr);
    dcacheMisses++;
    dcachePenalties += speed;
    
    uint32_t currentLRU = dcacheLRU[set_index][0];
    dcacheLRU[set_index][currentLRU] = tag;
    for (int k=0; k<dcacheAssoc-1; k++){
      dcacheLRU[set_index][k] = dcacheLRU[set_index][k+1];
    }
    dcacheLRU[set_index][dcacheAssoc-1] = currentLRU;
  }

  return speed;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  l2cacheRefs++;
  uint32_t speed = 0;

  int byte_offset = log2(blocksize);
  int tag_offset = log2(blocksize*l2cacheSets);

  uint32_t set_index = (addr >> byte_offset) & (l2cacheSets-1);
  uint32_t tag = addr >> tag_offset;

  int flag =  0;

  for (int i=0; i<l2cacheAssoc; i++){
    if (l2cache[set_index][i] == tag){
      speed = l2cacheHitTime;
      flag = 1;

      for (int j=0; j<l2cacheAssoc; j++){
        if (l2cacheLRU[set_index][j] == i){
          for (int k=j; k<l2cacheAssoc-1; k++){
            l2cacheLRU[set_index][k] = l2cacheLRU[set_index][k+1];
          }
          l2cacheLRU[set_index][l2cacheAssoc-1] = i;
          break;
        }
      }

      break;
    }
  }

  if (flag == 0){
    speed = l2cacheHitTime + memspeed;
    l2cacheMisses++;
    l2cachePenalties += speed;
    
    uint32_t currentLRU = l2cacheLRU[set_index][0];
    l2cacheLRU[set_index][currentLRU] = tag;
    for (int k=0; k<l2cacheAssoc-1; k++){
      l2cacheLRU[set_index][k] = l2cacheLRU[set_index][k+1];
    }
    l2cacheLRU[set_index][l2cacheAssoc-1] = currentLRU;
  }

  return speed;
}
