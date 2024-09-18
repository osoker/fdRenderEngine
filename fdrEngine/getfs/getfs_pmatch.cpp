#include "getfs_pmatch.h"


#if defined (__CYGWIN32__) || defined (WIN32)
#define FOLD_FN_CHAR(c) ( ((c < 'A') || (c > 'Z')) ? (c) : (c - 'A' + 'a') )
#else
#define FOLD_FN_CHAR(c) (c)
#endif

// Match STRING against the filename pattern PATTERN, returning zero if
// it matches, nonzero if not. 

int getfs_pmatch( char* _pattern, char* _string, int _flags )
{
    register char *p = _pattern, *n = _string;
    register char c;
   /* 
    if( ( _flags & ~__FNM_FLAGS ) != 0 )
	{
		//nfs_errno = nfs_EINVAL;
        return -1;
    }*/
    
    while( ( c = *p++ ) != '\0' )
	{
        switch( c ) 
		{
        case '?':
            if( *n == '\0' )
                return FNM_NOMATCH;
            else if( ( _flags & FNM_PATHNAME ) && *n == '/' )
                return FNM_NOMATCH;
            else if( ( _flags & FNM_PERIOD ) && *n == '.' &&
                ( n == _string || ( ( _flags & FNM_PATHNAME ) && n[-1] == '/' ) ) )
                return FNM_NOMATCH;
            break;
            
        case '\\':
            if( !( _flags & FNM_NOESCAPE ) )
                c = *p++;
            if( FOLD_FN_CHAR (*n) != FOLD_FN_CHAR (c) )
                return FNM_NOMATCH;
            break;
            
        case '*':
			{
				if( ( _flags & FNM_PERIOD) && *n == '.' &&
					( n == _string || ( ( _flags & FNM_PATHNAME) && n[-1] == '/' ) ) )
					return FNM_NOMATCH;

				if( _flags & FNM_ONLYCURFOLDERFILE )
				{
					char* n1 = n;
					for( ;*n1 != '\0';++n1 )
					{
						if( *n1 == '\\' )
							return FNM_NOMATCH;
					}
				}
	            
				for( c = *p++; c == '?' || c == '*'; c = *p++, ++n )
					if( ( ( _flags & FNM_PATHNAME ) && *n == '/' ) ||
						( c == '?' && *n == '\0' ) )
						return FNM_NOMATCH;
	                
					if( c == '\0' )
						return 0;
	                
					{
						char c1 = ( !( _flags & FNM_NOESCAPE ) && c == '\\') ? *p : c;
						for( --p; *n != '\0'; ++n )
							if( ( c == '[' || FOLD_FN_CHAR (*n) == FOLD_FN_CHAR (c1) ) &&
								getfs_pmatch( p, n, _flags & ~FNM_PERIOD ) == 0 )
								return 0;
							return FNM_NOMATCH;
					}
			}
                
        case '[':
            {
                // Nonzero if the sense of the character class is inverted.  
                register int not;
                
                if( *n == '\0' )
                    return FNM_NOMATCH;
                
                if( ( _flags & FNM_PERIOD) && *n == '.' &&
                    ( n == _string || ( ( _flags & FNM_PATHNAME ) && n[-1] == '/' ) ) )
                    return FNM_NOMATCH;
                
                not = ( *p == '!' || *p == '^' );
                if( not )
                    ++p;
                
                c = *p++;
                for(;;)
                {
                    register char cstart = c, cend = c;
                    
                    if( !( _flags & FNM_NOESCAPE) && c == '\\' )
                        cstart = cend = *p++;
                    
                    if( c == '\0' )
                        // [ (unterminated) loses.  
                        return FNM_NOMATCH;
                    
                    c = *p++;
                    
                    if( ( _flags & FNM_PATHNAME) && c == '/' )
                        // [/] can never match.  
                        return FNM_NOMATCH;
                    
                    if( c == '-' && *p != ']' )
                    {
                        cend = *p++;
                        if( !( _flags & FNM_NOESCAPE ) && cend == '\\' )
                            cend = *p++;
                        if( cend == '\0' )
                            return FNM_NOMATCH;
                        c = *p++;
                    }
                    
                    if( *n >= cstart && *n <= cend )
                        goto matched;
                    
                    if( c == ']' )
                        break;
                }
                if( !not )
                    return FNM_NOMATCH;
                break;
                
matched:;
        // Skip the rest of the [...] that already matched. 
        while( c != ']' )
        {
            if( c == '\0' )
                // [... (unterminated) loses. 
                return FNM_NOMATCH;
            
            c = *p++;
            if( !( _flags & FNM_NOESCAPE) && c == '\\' )
                // 1003.2d11 is unclear if this is right.  %%% 
                ++p;
        }
        if( not )
            return FNM_NOMATCH;
            }
            break;
            
        default:
            if (FOLD_FN_CHAR (c) != FOLD_FN_CHAR (*n))
                return FNM_NOMATCH;
    }
    
    ++n;
    }
    
    if( *n == '\0' )
        return 0;
    
    return FNM_NOMATCH;
}
