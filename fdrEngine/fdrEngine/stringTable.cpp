#include "stdafx.h"
# include "../../common/com_include.h"
#include "../interface/stringTable.h"


OSENGINE_API _StringTable *_gStringTable = NULL;
const unsigned int _StringTable::csm_stInitSize = 29;

//---------------------------------------------------------------
//
// StringTable functions
//
//---------------------------------------------------------------

namespace {
bool sgInitTable = true;
unsigned char   sgHashTable[256];

void initTolowerTable()
{
   for (unsigned int i = 0; i < 256; i++) {
      unsigned char c = tolower(i);
      sgHashTable[i] = c * c;
   }

   sgInitTable = false;
}

} // namespace {}

unsigned int _StringTable::hashString(const char* str)
{
   if (sgInitTable)
      initTolowerTable();

   if(!str) return -1;

   unsigned int ret = 0;
   char c;
   while((c = *str++) != 0) {
      ret <<= 1;
      ret ^= sgHashTable[c];
   }
   return ret;
}

unsigned int _StringTable::hashStringn(const char* str, int len)
{
   if (sgInitTable)
      initTolowerTable();

   unsigned int ret = 0;
   char c;
   while((c = *str++) != 0 && len--) {
      ret <<= 1;
      ret ^= sgHashTable[c];
   }
   return ret;
}

//--------------------------------------
_StringTable::_StringTable()
{
   buckets = (Node **) malloc(csm_stInitSize * sizeof(Node *));
   for(unsigned int i = 0; i < csm_stInitSize; i++) {
      buckets[i] = 0;
   }

   numBuckets = csm_stInitSize;
   itemCount = 0;
}

//--------------------------------------
_StringTable::~_StringTable()
{
	size_t i = 0;

	while(i<numBuckets){
		if(buckets[i]){
			Node* next = buckets[i]->next;
			while(next){
				free(next->val);
				Node* curr = next;
				next = next->next;
				free(curr);
			}
			free(buckets[i]->val);	
			free(buckets[i]);
		}
		i++;
	}
	
	free(buckets);
   
}


//--------------------------------------
void _StringTable::create()
{
   //AssertFatal(_gStringTable == NULL, "StringTable::create: StringTable already exists.");
   if(!_gStringTable)
      _gStringTable = new _StringTable;
}


//--------------------------------------
void _StringTable::destroy()
{
	// "delete NULL" cannot rise Exception
   //osassertex(StringTable != NULL, "StringTable::destroy: StringTable does not exist.");
   delete _gStringTable;
   _gStringTable = NULL;
}


//--------------------------------------
const char* _StringTable::insert(const char* _val, const bool caseSens)
{
   // Added 3/29/2007 -- If this is undesirable behavior, let me know -patw
   const char *val = _val;
   if( val == NULL )
      val = "";
   //-

   Node **walk, *temp;
   unsigned int key = hashString(val);
   walk = &buckets[key % numBuckets];
   while((temp = *walk) != NULL)   {
      if(caseSens && !strcmp(temp->val, val))
         return temp->val;
      else if(!caseSens && !_stricmp(temp->val, val))
         return temp->val;
      walk = &(temp->next);
   }
   char *ret = 0;
   if(!*walk) {
      *walk = (Node *) malloc(sizeof(Node));
      (*walk)->next = 0;
      (*walk)->val = (char *) malloc(strlen(val) + 1);
      strcpy((*walk)->val, val);
      ret = (*walk)->val;
      itemCount ++;
   }
   if(itemCount > 2 * numBuckets) {
      resize(4 * numBuckets - 1);
   }
   return ret;
}

//--------------------------------------
const char* _StringTable::insertn(const char* src, int len, const bool  caseSens)
{
   char val[256];
   osassertex(len < 255, "Invalid string to insertn");
   strncpy(val, src, len);
   val[len] = 0;
   return insert(val, caseSens);
}

//--------------------------------------
const char* _StringTable::lookup(const char* val, const bool  caseSens)
{
   Node **walk, *temp;
   unsigned int key = hashString(val);
   walk = &buckets[key % numBuckets];
   while((temp = *walk) != NULL)   {
      if(caseSens && !strcmp(temp->val, val))
            return temp->val;
      else if(!caseSens && !_stricmp(temp->val, val))
         return temp->val;
      walk = &(temp->next);
   }
   return NULL;
}

//--------------------------------------
const char* _StringTable::lookupn(const char* val, int len, const bool  caseSens)
{
   Node **walk, *temp;
   unsigned int key = hashStringn(val, len);
   walk = &buckets[key % numBuckets];
   while((temp = *walk) != NULL) {
      if(caseSens && !strncmp(temp->val, val, len) && temp->val[len] == 0)
         return temp->val;
      else if(!caseSens && !_strnicmp(temp->val, val, len) && temp->val[len] == 0)
         return temp->val;
      walk = &(temp->next);
   }
   return NULL;
}

//--------------------------------------
void _StringTable::resize(const unsigned int newSize)
{
   Node *head = NULL, *walk, *temp;
   unsigned int i;
   // reverse individual bucket lists
   // we do this because new strings are added at the end of bucket
   // lists so that case sens strings are always after their
   // corresponding case insens strings

   for(i = 0; i < numBuckets; i++) {
      walk = buckets[i];
      while(walk)
      {
         temp = walk->next;
         walk->next = head;
         head = walk;
         walk = temp;
      }
   }
   buckets = (Node **) realloc(buckets, newSize * sizeof(Node));
   for(i = 0; i < newSize; i++) {
      buckets[i] = 0;
   }
   numBuckets = newSize;
   walk = head;
   while(walk) {
      unsigned int key;
      Node *temp = walk;

      walk = walk->next;
      key = hashString(temp->val);
      temp->next = buckets[key % newSize];
      buckets[key % newSize] = temp;
   }
}

