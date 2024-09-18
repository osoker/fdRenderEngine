//Pattern matching for file names

#ifndef _GETFS_PMATCH_
#define _GETFS_PMATCH_

/* Bits set in the FLAGS argument to `fnmatch'.  */
#ifdef  FNM_PATHNAME
#undef  FNM_PATHNAME
#endif
#define	FNM_PATHNAME	(0 << 0) /* Wildcard can match `/' */

#ifdef  FNM_NOESCAPE
#undef  FNM_NOESCAPE
#endif
#define	FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars */

#ifdef  FNM_PERIOD
#undef  FNM_PERIOD
#endif
#define	FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly */

#ifdef FNM_ONLYCURFOLDERFILE
#undef FNM_ONLYCURFOLDERFILE
#endif
#define FNM_ONLYCURFOLDERFILE (1<<3)

#ifdef  __FNM_FLAGS
#undef  __FNM_FLAGS
#endif
#define	__FNM_FLAGS	( FNM_PATHNAME | FNM_NOESCAPE | FNM_PERIOD)
//#define	__FNM_FLAGS	(  FNM_PATHNAME | FNM_PERIOD  )

/* Value returned by 'fnmatch' if STRING does not match PATTERN.  */
#ifdef  FNM_NOMATCH
#undef  FNM_NOMATCH
#endif
#define	FNM_NOMATCH	1

/* Match STRING against the filename pattern PATTERN,
   returning zero if it matches, FNM_NOMATCH if not.  */
int getfs_pmatch( char* _pattern, char* _string, int _flags );

#endif //_GETFS_PMATCH_