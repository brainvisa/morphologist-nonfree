/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_preprocessing.c  * TYPE     : Source
 * AUTHOR      : MANGIN J.F.          * CREATION : 10/05/1996
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C                    * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : moulinette pour la generation des procedures typees
 *
 ****************************************************************************
 *
 * USED MODULES : 
 *
 *****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|----------------------------------------
 *              / /   |              |
 *****************************************************************************/

#define _XOPEN_SOURCE 500 /* for stdlib.h to define mkstemp */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_MAXLENGTH 1024
#define TYPE_NUMBER 7
#define TYPE_INT_NUMBER 5

#define TRUE 1
#define FALSE 0

#define EXTERN 1
#define STATIC 2

/*remove U32BIT to get rid of some warning in thresholding functions,
  do not want to use cast because of the possible (some short<100000) test*/
/*
const char type_template[TYPE_NUMBER][BUFFER_MAXLENGTH] =
    {{"U8BIT"},{"S8BIT"},{"U16BIT"},{"S16BIT"},{"U32BIT"},{"S32BIT"},
        {"FLOAT"},{"DOUBLE"}};

const char type_template_int[TYPE_INT_NUMBER][BUFFER_MAXLENGTH] =
    {{"U8BIT"},{"S8BIT"},{"U16BIT"},{"S16BIT"},{"U32BIT"},{"S32BIT"}};
*/
const char type_template[TYPE_NUMBER][BUFFER_MAXLENGTH] =
    {{"U8BIT"},{"S8BIT"},{"U16BIT"},{"S16BIT"},{"S32BIT"},
     {"FLOAT"},{"DOUBLE"}};

const char type_template_int[TYPE_INT_NUMBER][BUFFER_MAXLENGTH] =
    {{"U8BIT"},{"S8BIT"},{"U16BIT"},{"S16BIT"},{"S32BIT"}};

#ifdef _WIN32
const char VipTmpFile[] = "C:\\windows\\temp\\temp_";
const char VipTmpHFile[] = "C:\\windows\\temp\\temp_h_";
const char sep[] = "\\";
#else
const char VipTmpFile[] = "/tmp/temp_";
const char VipTmpHFile[] = "/tmp/temp_h_";
const char sep[] = "/";
#endif
 
/*----------------------------------------------------------------------------*/
static void		PreVipPrintfExit( char *fname);
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
static void PreVipPrintfError ( char *s );
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static void			PreVipPrintfExitInFunction( char *fname);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static void PreVipPrintfWarning (char *s);
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static void TypedExpansion(
char type_name[TYPE_NUMBER][BUFFER_MAXLENGTH],
FILE *type_file[TYPE_NUMBER],
char type_filename[TYPE_NUMBER][BUFFER_MAXLENGTH],
int func_ntype,
char *buffer,
int flag);
/*----------------------------------------------------------------------------*/

static FILE* maketemp( char* template )
{
#ifdef _WIN32
    if( mktemp( template ) == 0 )
        return NULL;
    return fopen( template, "w" );

#else
    int op = mkstemp( template );
    if( op == -1 )
        return NULL;
    return fdopen( op, "w" );
    
#endif
}


int makedir( const char* pathname )
{
#ifdef _WIN32
    return mkdir( pathname );
#else
    return mkdir( pathname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
#endif
}

int filecat( const char ** infile,
             const int     count,
             const char *  outfile,
             const char *  outfile_mode)
{
    FILE *fs, *ft;
    const char * m;
    char ch;
    int i;
    
    if (outfile_mode == NULL)
        m = "a";        
    else
        m = outfile_mode;
    
    ft = fopen(outfile, m);
    if( ft == NULL )
    {
        perror("VipPreprocessing: Unable to open output file in write mode.");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < count; ++i) {
        if (infile[i] != NULL){
            if (access(infile[i], F_OK) != -1)
                fs = fopen(infile[i], "r");
        }

        if((fs == NULL) && (infile[i] != NULL))
        {
            perror("VipPreprocessing: Unable to open one of input files in read mode.");
            exit(EXIT_FAILURE);
        }

        if (fs != NULL) {
            while( ( ch = fgetc(fs) ) != EOF )
                fputc(ch, ft);
            fclose(fs);
            fs = NULL;
        }
    }
    fclose(ft);
    return 0;
}

int main(int argc, char *argv[])
{       
    int debug_flag = FALSE;
    int debug_file = FALSE;

    char *gen_filename;
    char c_filename[BUFFER_MAXLENGTH];
    char externh_filename[BUFFER_MAXLENGTH];
    char statich_filename[BUFFER_MAXLENGTH];
    char name[BUFFER_MAXLENGTH];
    FILE *gen_file, *c_file, *externh_file, *statich_file;
    /* FILE *dirinclude; */
    int l;
    char *search;
    char buffer[BUFFER_MAXLENGTH];
    int prototype_flag, typed_func_flag, body_func_flag, type_list;
    int nontyped_func_flag=0;
    int the_extern_static=0;
    int n_type_list;
    int extern_static_flag[TYPE_NUMBER];
    char func_name[BUFFER_MAXLENGTH];
    int func_ntype, old_func_ntype;
    int count;
    char error[BUFFER_MAXLENGTH];
    char field1[BUFFER_MAXLENGTH];
    int n;
    char temp_filename[TYPE_NUMBER][BUFFER_MAXLENGTH];
    FILE *temp_file[TYPE_NUMBER];
    char temp_hfilename[TYPE_NUMBER][BUFFER_MAXLENGTH];
    FILE *temp_hfile[TYPE_NUMBER];
    int i,j;
    char type_name[TYPE_NUMBER][BUFFER_MAXLENGTH];
    char *bufferptr;
    int prototype_count;
    int body_count;
    char command[BUFFER_MAXLENGTH];
#ifdef _WIN32
    char dirinclude_name[BUFFER_MAXLENGTH]=".\\";
#else
    char dirinclude_name[BUFFER_MAXLENGTH]="./";
#endif
    int nontyped_expansion;
    int h_expansion;

    /*  parametres en lignes */
    if(argc!=2)
    {   
        PreVipPrintfExit("Usage:  Vip_preprocessing <file.gen>");
    }
    gen_filename = argv[1];
    l=strlen(gen_filename);
    search = strstr(gen_filename,".gen");

    if((gen_filename==NULL)||(l==0)||(l<=4)||strcmp(gen_filename+l-4,".gen")
        ||(search!=gen_filename+l-4))
    {
        (void)sprintf(error,"Incoherent generic filename: %s (.gen extension required)", gen_filename);
        PreVipPrintfExit(error);
    }

    (void)strcpy(name,gen_filename);
    name[l-4] = '\0';
    if(debug_flag==TRUE)
    {
        (void)printf("\nCreating typed functions for %s module:\n\n",name);
    }
    gen_file = fopen(gen_filename,"r");
    if(debug_file==TRUE) printf("fopen %s r\n",gen_filename);

    if(!gen_file)
    {
        sprintf(error,"Can not open %s for reading\n",gen_filename);
        PreVipPrintfExit(error);
    }

    strcpy(c_filename,name);
    strcat(c_filename,"_gen.c");
    c_file = fopen(c_filename,"w");
    if(debug_file==TRUE) printf("fopen %s w\n",c_filename);

    if(!c_file)
    {
        sprintf(error,"Can not open %s for writing\n",c_filename);
        PreVipPrintfExit(error);

    }

    /* dirinclude = fopen(dirinclude_name,"r");
    printf( "dirinclude: %s, open: %p\n", dirinclude_name, dirinclude );
    if(!dirinclude && strcmp( dirinclude_name, "./" ) 
        && strcmp( dirinclude_name, ".\\" ) )
    {
        sprintf(error,"No '%s' directory, creating it...",dirinclude_name);
        PreVipPrintfWarning(error);
        makedir( dirinclude_name );
    }
    else (void)fclose(dirinclude); */

    strcpy(externh_filename,dirinclude_name);
    strcat(externh_filename,sep);
    strcat(externh_filename,name);
    strcat(externh_filename,"_gen.h");
    externh_file = fopen(externh_filename,"w");
    if(debug_file==TRUE) printf("fopen %s w\n",externh_filename);

    if(!externh_file)
    {
        sprintf(error,"Can not open %s for writing\n",externh_filename);
        PreVipPrintfExit(error);
    }

    strcpy(statich_filename,dirinclude_name);
    strcat(statich_filename,sep);
    strcat(statich_filename,name);
    strcat(statich_filename,"_gen_static.h");
    statich_file = fopen(statich_filename,"w");
    if(debug_file==TRUE) printf("fopen %s w\n",statich_filename);
    if(!statich_file)
    {
        sprintf(error,"Can not open %s for writing\n",statich_filename);
        PreVipPrintfExit(error);
    }

    prototype_flag = FALSE;
    typed_func_flag = FALSE;
    body_func_flag = FALSE;
    type_list = FALSE;
    n_type_list = 0;
    func_ntype = 0;
    old_func_ntype = 0;
    count = 1;
    prototype_count =0;
    body_count =0;
    nontyped_expansion = FALSE;
    h_expansion = FALSE;


    for(i=0;i<TYPE_NUMBER;i++)
    {
        extern_static_flag[i] = FALSE;
        temp_hfile[i] = NULL;
        temp_file[i] = NULL;
    }

    if ( !fgets(buffer, BUFFER_MAXLENGTH, gen_file) )
    {
        sprintf(error,"File %s is corrupted\n",statich_filename);
        PreVipPrintfExit(error);
    }

    for(;!feof(gen_file);count++)
    {
        if(*buffer=='#')
        {
            if(buffer[1]=='H')
            {
                if((search = strstr(buffer,"H_EXPAND_BEGIN"))
                    &&(search==(buffer+1)))
                {
                    if(h_expansion==TRUE)
                    {
                        PreVipPrintfError("Two consecutive H_EXPAND_BEGIN");
                        PreVipPrintfExitInFunction("H_EXPAND");
                    }
                    h_expansion = TRUE;
                }
                else if((search = strstr(buffer,"H_EXPAND_END"))
                        &&(search==(buffer+1)))
                {
                    if(h_expansion==FALSE)
                    {
                        PreVipPrintfError("H_EXPAND_END without H_EXPAND_BEGIN");
                        PreVipPrintfExitInFunction("H_EXPAND");
                    }
                    
                    for(i=0;i<func_ntype;i++)
                    {
                        if( temp_hfile[i] != NULL )
                        {
                            (void)fclose(temp_hfile[i]);
                            if(debug_file==TRUE) printf("fclose 1 (hexpand_end) %s w\n",temp_hfilename[i]);
                            (void)fclose(temp_file[i]);
                            if(debug_file==TRUE) printf("fclose 1 (hexpand_end) %s w\n",temp_filename[i]);
                        }
                    }
                    if(n_type_list==1)
                    {
                        if(extern_static_flag[0]==EXTERN)
                            (void)fclose(externh_file);
                        else
                            (void)fclose(statich_file);
                    }
                    else
                    {
                        (void)fclose(externh_file);
                        (void)fclose(statich_file);
                    }
                    
                    if(((n_type_list==1)&&(extern_static_flag[0]==EXTERN))||(n_type_list==2))
                    {
                        int file_count = 0;
                        char * files[TYPE_NUMBER];
                        for(i=0;i<func_ntype;i++)
                        {
                            if(extern_static_flag[i]==EXTERN)
                            {
                                files[file_count] = temp_hfilename[i];
                                ++file_count;
                            }
                        }
                        filecat( (const char**)&files[0], file_count, externh_filename, NULL );
                    }
                    
                    if(((n_type_list==1)&&(extern_static_flag[0]==STATIC))||(n_type_list==2))
                    {
                        int file_count = 0;
                        char * files[TYPE_NUMBER];
                        for(i=0;i<func_ntype;i++)
                        {
                            if(extern_static_flag[i]==STATIC)
                            {
                                files[file_count] = temp_hfilename[i];
                                ++file_count;
                            }
                        }
                        filecat( (const char**)&files[0], file_count, statich_filename, NULL );
                    }
                    if(n_type_list==1)
                    {
                        if(extern_static_flag[0]==EXTERN)
                        {
                            externh_file = fopen(externh_filename,"a");
                            if(!externh_file)
                            {
                                sprintf(error,"Can not open %s for writing\n",externh_filename);
                                PreVipPrintfExit(error);
                            }
                        }
                        else 
                        {
                            statich_file = fopen(statich_filename,"a");
                            if(!statich_file)
                            {
                                sprintf(error,"Can not open %s for writing\n",statich_filename);
                                PreVipPrintfExit(error);
                            }
                        }
                    }
                    else
                    {
                        externh_file = fopen(externh_filename,"a");
                        if(!externh_file)
                        {
                            sprintf(error,"Can not open %s for writing\n",externh_filename);
                            PreVipPrintfExit(error);
                        }
                        
                        statich_file = fopen(statich_filename,"a");
                        if(!statich_file)
                        {
                            sprintf(error,"Can not open %s for writing\n",statich_filename);
                            PreVipPrintfExit(error);
                        }
                    }  
                    
                    for(i=0;i<func_ntype;i++)
                    {
                        unlink( temp_hfilename[i] );
                        temp_hfile[i] = NULL;
                        if(debug_file==TRUE) printf("unlink 1 (hexpand_end) %s w\n",temp_hfilename[i]);
                        unlink( temp_filename[i] );
                        temp_file[i] = NULL;
                        if(debug_file==TRUE) printf("unlink 1 (hexpand_end) %s w\n",temp_filename[i]);
                    }

                    h_expansion = FALSE;
                    n_type_list = 0;
                    old_func_ntype=0;
                    func_ntype = 0;
                }
            }
            else if(buffer[1]=='T')
            {
                if((search = strstr(buffer,"TYPED_FUNC_BEGIN"))
                    &&(search==(buffer+1)))
                {
                    if(typed_func_flag==TRUE)
                    {
                        PreVipPrintfError("Two consecutive TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    else
                    {
                        typed_func_flag=TRUE;
                        n = sscanf(buffer,"%s%s",field1,func_name);
                        if(n!=2)
                        {
                            PreVipPrintfError("TYPED_FUNC_BEGIN has to introduce generic function name");
                            (void)fprintf(stderr,"Ex: #TYPED_FUNC_BEGIN VipGetDataPtr_GENSTRING\n");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                        else
                        {
                            search = strstr(func_name,"GENSTRING");
                            if(search==NULL)
                            {
                                PreVipPrintfError("Typed generic function name has to include GENSTRING");
                                (void)sprintf(error,"%s (line %d)",func_name,count);
                                PreVipPrintfExitInFunction(error);
                            }
                            search = strstr(search+1,"GENSTRING");
                            if(search!=NULL)
                            {
                                PreVipPrintfError("Typed generic function name has to include only one GENSTRING");
                                (void)sprintf(error,"%s (line %d)",func_name,count);
                                PreVipPrintfExitInFunction(error);
                            }
                            if(debug_flag==TRUE)
                            {
                                (void)printf(" Begin expansion of %s:\n",func_name);
                            }
                        }
                    }
                }
                else if((search = strstr(buffer,"TYPED_FUNC_END"))
                         &&(search==(buffer+1)))

                {
                    if(typed_func_flag==FALSE)
                    {
                        PreVipPrintfError("TYPED_FUNC_END without TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(body_func_flag==TRUE)
                    {
                        PreVipPrintfError("TYPED_FUNC_END without FUNCTION_END");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(prototype_count==FALSE)
                    {
                        if(prototype_flag==FALSE)
                        {
                            PreVipPrintfError("Function without prototype");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                        else
                        {
                            PreVipPrintfError("prototype without PROTOTYPE_END");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                    }
                    if(body_count==FALSE)
                    {
                        if(body_func_flag==FALSE)
                        {
                            PreVipPrintfError("Function without body");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                            else
                        {
                            PreVipPrintfError("body without FUNCTION_END");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                    }
                    else
                    {
                        typed_func_flag=FALSE;
                        type_list = FALSE;
                        n_type_list = 0;
                        old_func_ntype=0;
                        prototype_count = FALSE;
                        body_count = FALSE;
                        char * files[TYPE_NUMBER];
                        for(i=0;i<func_ntype;i++)
                        {
                            files[i] = temp_filename[i];
                            (void)fprintf(temp_file[i],"\n");
                            (void)fclose(temp_file[i]);
                            if(debug_file==TRUE) printf("fclose 2 (typed func) %s \n",temp_filename[i]);
                        }
                        (void)fclose(c_file);

                        filecat( (const char**)&files[0], func_ntype, c_filename, NULL );
                    
                        c_file = fopen(c_filename,"a");
                        if(debug_file==TRUE) printf("fopen %s a\n",c_filename);

                        if(!c_file)
                        {
                            sprintf(error,"Can not open %s for writing\n",c_filename);
                            PreVipPrintfExit(error);
                        }
                        for(i=0;i<func_ntype;i++)
                        {
                            unlink( temp_filename[i] );
                            if(debug_file==TRUE) printf("unlink 2 (typed func) %s \n",temp_filename[i]);
                            temp_file[i] = 0;
                        }
                        func_ntype = 0;
                        if(debug_flag==TRUE)
                        {
                            (void)printf(" End\n");
                        }
                    }
                }
                else if((search = strstr(buffer,"TYPE_LIST"))
                         &&(search==(buffer+1)))
                {
                    if((typed_func_flag==FALSE)&&(nontyped_func_flag==FALSE)&&(h_expansion==FALSE))
                    {
                        PreVipPrintfError("TYPE_LIST without [NON]TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(n_type_list==2)
                    {
                        PreVipPrintfError("already two TYPE_LIST_XXX found");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    else
                    {
                        type_list = TRUE;
                        n_type_list++;
                        if (n_type_list==1 ) func_ntype=0;
                        else old_func_ntype = func_ntype;
                        bufferptr = buffer;

                        if((typed_func_flag==TRUE)||(h_expansion==TRUE))
                        {
                            search = strstr(buffer,"TYPE_LIST_EXTERN");
                            if(search!=NULL) the_extern_static = EXTERN;
                            else
                            {
                                search = strstr(buffer,"TYPE_LIST_STATIC");
                                if(search!=NULL) the_extern_static = STATIC;
                                else
                                {
                                    PreVipPrintfError("syntax: TYPE_LIST_EXTERN or TYPE_LIST_STATIC");
                                    (void)sprintf(error,"%s (line %d)",func_name,count);
                                    PreVipPrintfExitInFunction(error);
                                }
                            }
                        }

                        while(func_ntype<TYPE_NUMBER)
                        {
                            n = sscanf(bufferptr,"%s%s",field1,type_name[func_ntype]);
                            if(n!=2)
                            {
                                if(((func_ntype==0)&&(n_type_list==1))||((func_ntype==old_func_ntype)&&(n_type_list==2)))
                                {
                                    PreVipPrintfError("no type after TYPE_LIST[_XXX]");
                                    (void)sprintf(error,"%s (line %d)",func_name,count);
                                    PreVipPrintfExitInFunction(error);
                                }
                                else break;
                            }
                            else
                            {
                                extern_static_flag[func_ntype] = the_extern_static;
                                func_ntype++;
                                bufferptr+=strlen(field1)+1;
                            }
                        }
                
                        if(!strcmp(type_name[old_func_ntype],"ANY") && (((func_ntype>1) && (n_type_list==1))||
                                                ((func_ntype>(old_func_ntype+1)) && (n_type_list==2))))
                        {
                            PreVipPrintfError("ANY type followed by something else?");
                            (void)sprintf(error,"%s (line %d)",func_name,count);
                            PreVipPrintfExitInFunction(error);
                        }
                        
                        if(!strcmp(type_name[old_func_ntype],"ANY_INT") && (((func_ntype>1) && (n_type_list==1))||
                                                ((func_ntype>(old_func_ntype+1)) && (n_type_list==2))))
                        {
                            PreVipPrintfError("ANY_INT type followed by something else?");
                            (void)sprintf(error,"%s (line %d)",func_name,count);
                            PreVipPrintfExitInFunction(error);
                        }
                        if(!strcmp(type_name[old_func_ntype],"ANY"))
                        {
                            if(n_type_list==1)
                            {
                                func_ntype=TYPE_NUMBER;
                                for(i=0;i<func_ntype;i++) 
                                {
                                    (void)strcpy(type_name[i],type_template[i]);
                                    extern_static_flag[i] = extern_static_flag[old_func_ntype];
                                }
                            }
                            else
                            {
                                PreVipPrintfError("Repeated types");
                                (void)sprintf(error,"%s (line %d)",func_name,count);
                                PreVipPrintfExitInFunction(error);
                            }
                        }
                        if(!strcmp(type_name[old_func_ntype],"ANY_INT"))
                        {
                            if(n_type_list==1)
                            {
                                func_ntype=TYPE_INT_NUMBER;
                                for(i=0;i<func_ntype;i++) 
                                {
                                    (void)strcpy(type_name[i],type_template_int[i]);
                                    extern_static_flag[i] = extern_static_flag[old_func_ntype];
                                }
                            }
                            else 
                            {
                                if (old_func_ntype>2)
                                {
                                    PreVipPrintfError("Repeated types");
                                    (void)sprintf(error,"%s (line %d)",func_name,count);
                                    PreVipPrintfExitInFunction(error);
                                }
                                else
                                {
                                    func_ntype=old_func_ntype + TYPE_INT_NUMBER;
                                    for(i=old_func_ntype;i<func_ntype;i++) 
                                    {
                                        (void)strcpy(type_name[i],type_template_int[i-old_func_ntype]);
                                        extern_static_flag[i] = extern_static_flag[old_func_ntype];
                                    }
                                }
                            }
                        }

                        if((func_ntype>TYPE_NUMBER)||((n_type_list==2)&&(func_ntype==old_func_ntype)))
                        {
                            PreVipPrintfError("Repeated types");
                            (void)sprintf(error,"%s (line %d)",func_name,count);
                            PreVipPrintfExitInFunction(error);
                        }
                        if(debug_flag==TRUE)
                        {
                            if (extern_static_flag[old_func_ntype]==EXTERN)
                            (void)printf("  public Types: ");
                            else
                            (void)printf("  private Types: ");
                            for(i=old_func_ntype;i<func_ntype;i++) (void)printf("%s ",type_name[i]);
                            (void)printf("\n");
                        }
            
                        for(i=old_func_ntype;i<func_ntype;i++) 
                        {
                            for(j=0;j<TYPE_NUMBER;j++)
                            {
                                if(!strcmp(type_name[i],type_template[j])) break;
                            }
                            if(j==TYPE_NUMBER)
                            {
                                (void)sprintf(error,"Unknown type: %s",type_name[i]);
                                PreVipPrintfError(error);
                                (void)sprintf(error,"%s (line %d)",func_name,count);
                                PreVipPrintfExitInFunction(error);
                            }
                            if( temp_file[i] != NULL )
                            {
                                            fclose( temp_file[i] );
                                unlink( temp_filename[i] );
                                            if(debug_file==TRUE) printf("fclose/unlink 3 (type_list, BUG) %s \n",temp_filename[i]);
                                temp_file[i] = NULL;
                                printf( "cfile %s should not exist...\n", temp_filename[i] );
                            }
                            (void)strcpy(temp_filename[i],VipTmpFile);
                            /* (void)strcat(temp_filename[i],name);*/
                            (void)strcat(temp_filename[i],type_name[i]);
                            (void)strcat(temp_filename[i],"XXXXXX");
                            if( temp_file[i] != NULL )
                                printf( "overwriting file descriptor of %s\n", temp_filename[i] );
                            temp_file[i] = maketemp( temp_filename[i] );
                            if(debug_file==TRUE) printf("fopen (type_list) %s w\n",temp_filename[i]);
                            if(!temp_file[i])
                            {
                                (void)sprintf(error,"Can not create temp .c file %s",
                                    temp_filename[i]);
                                PreVipPrintfExit(error);
                            }

                            if((typed_func_flag==TRUE)||(h_expansion==TRUE))
                            {
                                if( temp_hfile[i] != NULL )
                                {
                                    fclose( temp_hfile[i] );
                                    unlink( temp_hfilename[i] );
                                    if(debug_file==TRUE) printf("fclose/unlink 3 %s \n",temp_hfilename[i]);
                                    temp_hfile[i] = NULL;
                                    printf( "cfile %s should not exist...\n", temp_hfilename[i] );
                                }
                                (void)strcpy(temp_hfilename[i],VipTmpHFile);
                                /*(void)strcat(temp_hfilename[i],name);*/
                                (void)strcat(temp_hfilename[i],type_name[i]);
                                (void)strcat(temp_hfilename[i],"XXXXXX");
                                temp_hfile[i] = maketemp( temp_hfilename[i] );
                                if(debug_file==TRUE) printf("fopen (type_list) %s w\n",temp_hfilename[i]);
                                if(!temp_hfile[i])
                                {
                                    (void)sprintf(error,"Can not create temp .h file %s for writing",
                                        temp_hfilename[i]);
                                    PreVipPrintfExit(error);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(typed_func_flag==TRUE)
                    { 
                        TypedExpansion(type_name,temp_file,temp_filename,
                                       func_ntype,buffer,type_list);

                        if(prototype_flag==TRUE)
                            TypedExpansion(type_name,temp_hfile,temp_hfilename,
                                           func_ntype,buffer,type_list);
                    }
                    else
                    {
                        n = fprintf(c_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",c_filename);
                            PreVipPrintfExit(error);
                        }
                    }
                }
            }
            else if(buffer[1]=='N')
            {
                if((search = strstr(buffer,"NONTYPED_FUNC_BEGIN"))
                    &&(search==(buffer+1)))
                {
                    if(nontyped_func_flag==TRUE)
                    {
                        PreVipPrintfError("Two consecutive NONTYPED_FUNC_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    else
                    {
                        nontyped_func_flag=TRUE;
                        n = sscanf(buffer,"%s%s",field1,func_name);
                        if(n!=2)
                        {
                            PreVipPrintfError("NONTYPED_FUNC_BEGIN has to introduce generic function name");
                            (void)fprintf(stderr,"Ex: #NONTYPED_FUNC_BEGIN VipSetVolumeLevel\n");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                        if(debug_flag==TRUE)
                        {
                            (void)printf(" Begin nontyped expansion of %s:\n",func_name);
                        }
                    }
                }
                else if((search = strstr(buffer,"NONTYPED_FUNC_END"))
                        &&(search==(buffer+1)))
                {
                    if(nontyped_func_flag==FALSE)
                    {
                        PreVipPrintfError("NONTYPED_FUNC_END without NONTYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(body_func_flag==TRUE)
                    {
                        PreVipPrintfError("NONTYPED_FUNC_END without FUNCTION_END");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(prototype_count==FALSE)
                    {
                        if(prototype_flag==FALSE)
                        {
                            PreVipPrintfError("Function without prototype");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                        else
                        {
                            PreVipPrintfError("prototype without PROTOTYPE_END");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                    }
                    if(body_count==FALSE)
                    {
                        if(body_func_flag==FALSE)
                        {
                            PreVipPrintfError("Function without body");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                        else
                        {
                            PreVipPrintfError("body without FUNCTION_END");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                    }
                    else
                    {
                        nontyped_func_flag=FALSE;
                        type_list = FALSE;
                        n_type_list = 0;
                        old_func_ntype=0;
                        prototype_count = FALSE;
                        body_count = FALSE;
                        
                        
                        func_ntype = 0;
                        if(debug_flag==TRUE)
                        {
                            (void)printf(" End\n");
                        }
                    }
                }
                else
                {
                    if(typed_func_flag==TRUE)
                    {
                        TypedExpansion(type_name,temp_file,temp_filename,
                                    func_ntype,buffer,type_list);
                        if(prototype_flag==TRUE)
                            TypedExpansion(type_name,temp_hfile,temp_hfilename,
                                        func_ntype,buffer,type_list);
                    }
                    else if (!strstr(buffer,"NONTYPED_FUNC_BEGIN"))
                    {
                        n = fprintf(c_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",c_filename);
                            PreVipPrintfExit(error);
                        }
                    }
                }
            }
            else if(buffer[1]=='P')
            {
                if((search = strstr(buffer,"PROTOTYPE_BEGIN"))
                    &&(search==(buffer+1)))
                {
                    if((typed_func_flag==FALSE)&&(nontyped_func_flag==FALSE))
                    {
                        PreVipPrintfError("PROTOTYPE_BEGIN without [NON]TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(type_list==FALSE)
                    {
                        PreVipPrintfError("PROTOTYPE_BEGIN without TYPE_LIST");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    if(prototype_flag==TRUE)
                    {
                        PreVipPrintfError("Two consecutive PROTOTYPE_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    if(prototype_count!=FALSE)
                    {
                        PreVipPrintfError("Two successive prototypes");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    } 
                    else
                    {
                        if(debug_flag==TRUE)
                        {
                            (void)printf("  PROTOTYPE_BEGIN\n");
                        }
                        prototype_flag=TRUE;
                    }
                }
                else if((search = strstr(buffer,"PROTOTYPE_END"))
                        &&(search==(buffer+1)))
                {
                    if((typed_func_flag==FALSE)&&(nontyped_func_flag==FALSE))
                    {
                        PreVipPrintfError("PROTOTYPE_END without [NON]TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(prototype_flag==FALSE)
                    {
                        PreVipPrintfError("PROTOTYPE_END without PROTOTYPE_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);					
                    }
                    else
                    {
                        if(debug_flag==TRUE)
                        {
                            (void)printf("  PROTOTYPE_END\n");
                        }
                        prototype_flag=FALSE;
                        prototype_count = TRUE;
                    }
                    if(type_list==FALSE)
                    {
                        PreVipPrintfError("PROTOTYPE_END without TYPE_LIST");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    else if (typed_func_flag==TRUE)
                    {
                        for(i=0;i<func_ntype;i++)
                        {
                            fprintf(temp_hfile[i],";\n");
                            (void)fclose(temp_hfile[i]);
                            if(debug_file==TRUE) printf("fclose 4 (proto_end) %s w\n",temp_hfilename[i]);

                        }

                        if(n_type_list==1)
                        {
                            if(extern_static_flag[0]==EXTERN)
                                (void)fclose(externh_file);
                            else 
                                (void)fclose(statich_file);
                        }
                        else
                        {
                            (void)fclose(externh_file);
                            (void)fclose(statich_file);
                        }

                        if(((n_type_list==1)&&(extern_static_flag[0]==EXTERN))||(n_type_list==2))
                        {
                            int file_count = 0;
                            char * files[TYPE_NUMBER];
                            for(i=0;i<func_ntype;i++)
                            {
                                if(extern_static_flag[i]==EXTERN)
                                {
                                    files[file_count] = temp_hfilename[i];
                                    ++file_count;
                                }
                            }
                            filecat( (const char**)&files[0], file_count, externh_filename, NULL );
                        }

                        if(((n_type_list==1)&&(extern_static_flag[0]==STATIC))||(n_type_list==2))
                        {
                            int file_count = 0;
                            char * files[TYPE_NUMBER];
                            for(i=0;i<func_ntype;i++)
                            {
                                if(extern_static_flag[i]==STATIC)
                                {
                                    files[file_count] = temp_hfilename[i];
                                    ++file_count;
                                }
                            }
                            filecat( (const char**)&files[0], file_count, statich_filename, NULL );
                        }
                        if(n_type_list==1)
                        {
                            if(extern_static_flag[0]==EXTERN)
                            {
                                externh_file = fopen(externh_filename,"a");
                                if(!externh_file)
                                {
                                    sprintf(error,"Can not open %s for writing\n",externh_filename);
                                    PreVipPrintfExit(error);
                                }
                            }
                            else 
                            {
                                statich_file = fopen(statich_filename,"a");
                                if(!statich_file)
                                {
                                    sprintf(error,"Can not open %s for writing\n",statich_filename);
                                    PreVipPrintfExit(error);
                                }
                            }
                        }
                        else
                        {
                            externh_file = fopen(externh_filename,"a");
                            if(!externh_file)
                            {
                                sprintf(error,"Can not open %s for writing\n",externh_filename);
                                PreVipPrintfExit(error);
                            }
                            
                            statich_file = fopen(statich_filename,"a");
                            if(!statich_file)
                            {
                                sprintf(error,"Can not open %s for writing\n",statich_filename);
                                PreVipPrintfExit(error);
                            }
                        }      

                        for(i=0;i<func_ntype;i++)
                            if( temp_hfilename[i] )
                            {
                                unlink( temp_hfilename[i] );
                                temp_hfile[i] = NULL;
                                if(debug_file==TRUE) printf("unlink 4 (proto_end) %s w\n",temp_hfilename[i]);
                            }
                    }
                }
                else
                {
                    if(typed_func_flag==TRUE)
                    {
                        TypedExpansion(type_name,temp_file,temp_filename,
                                    func_ntype,buffer,type_list);
                        if(prototype_flag==TRUE)
                            TypedExpansion(type_name,temp_hfile,temp_hfilename,
                                        func_ntype,buffer,type_list);
                    }
                    else
                    {
                        n = fprintf(c_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",c_filename);
                            PreVipPrintfExit(error);
                        }
                    }

                }
            }
            else if(buffer[1]=='F')
            {
                if((search = strstr(buffer,"FUNCTION_BEGIN"))
                    &&(search==(buffer+1)))
                {
                    if((typed_func_flag==FALSE)&&(nontyped_func_flag==FALSE))
                    {
                        PreVipPrintfError("FUNCTION_BEGIN without [NON]TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(type_list==FALSE)
                    {
                        PreVipPrintfError("FUNCTION_BEGIN without TYPE_LIST");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    if(prototype_flag==TRUE)
                    {
                        PreVipPrintfError("FUNCTION_BEGIN before PROTOTYPE_END");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);		
                    }
                    if(prototype_count==FALSE)
                    {
                        PreVipPrintfError("FUNCTION_BEGIN before prototype");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);		
                    }
                    if(body_func_flag==TRUE)
                    {
                        PreVipPrintfError("Two consecutive FUNCTION_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);		
                    }
                    if(body_count!=FALSE)
                    {
                        PreVipPrintfError("Two consecutive body functions");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);		
                    }
                    else
                    {
                        if(debug_flag==TRUE)
                        {
                            (void)printf("  FUNCTION BEGIN\n");
                        }
                        body_func_flag=TRUE;
                    }
                }
                else if((search = strstr(buffer,"FUNCTION_END"))
                        &&(search==(buffer+1)))
                {
                    if((typed_func_flag==FALSE)&&(nontyped_func_flag==FALSE))
                    {
                        PreVipPrintfError("FUNCTION_END without [NON]TYPED_FUNC_BEGIN");
                        (void)sprintf(error,"(line %d)",count);
                        PreVipPrintfExit(error);
                    }
                    if(type_list==FALSE)
                    {
                        PreVipPrintfError("FUNCTION_END without TYPE_LIST");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);
                    }
                    if(prototype_flag==TRUE)
                    {
                        PreVipPrintfError("FUNCTION_END before PROTOTYPE_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);		
                    }
                    if(body_func_flag==FALSE)
                    {
                        PreVipPrintfError("FUNCTION_END without FUNCTION_BEGIN");
                        (void)sprintf(error,"%s (line %d)",func_name,count);
                        PreVipPrintfExitInFunction(error);		
                    }
                    else
                    {
                        if(debug_flag==TRUE)
                        {
                            (void)printf("  FUNCTION_END\n");
                        }
                        body_func_flag=FALSE;
                        body_count = TRUE;
                    }
                }
                else
                {
                    if(typed_func_flag==TRUE)
                    {
                        TypedExpansion(type_name,temp_file,temp_filename,
                                    func_ntype,buffer,type_list);
                        if(prototype_flag==TRUE)
                            TypedExpansion(type_name,temp_hfile,temp_hfilename,
                                        func_ntype,buffer,type_list);
                    }
                    else
                    {
                        n = fprintf(c_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",c_filename);
                            PreVipPrintfExit(error);
                        }
                    }
                }
            }
            else
            {
                if(typed_func_flag==TRUE)
                {
                    TypedExpansion(type_name,temp_file,temp_filename,
                                func_ntype,buffer,type_list);
                    if(prototype_flag==TRUE)
                        TypedExpansion(type_name,temp_hfile,temp_hfilename,
                                    func_ntype,buffer,type_list);
                }
                else
                {
                    n = fprintf(c_file,"%s",buffer);
                    if(n!=(int)strlen(buffer))
                    {
                        (void)sprintf(error,"Can not write correctly in %s",c_filename);
                        PreVipPrintfExit(error);
                    }
                }
            }
        }
        else
        {
            if(typed_func_flag==TRUE)
            {
                TypedExpansion(type_name,temp_file,temp_filename,
                            func_ntype,buffer,type_list);
                if(prototype_flag==TRUE)
            
                TypedExpansion(type_name,temp_hfile,temp_hfilename,
                            func_ntype,buffer,type_list);
            }
            else if(h_expansion==TRUE)
            {
                TypedExpansion(type_name,temp_hfile,temp_hfilename,
                            func_ntype,buffer,type_list);
            }
            else
            {
                if (nontyped_func_flag==TRUE)
                {
                    if(prototype_flag==TRUE)
                    {
                        n = fprintf(externh_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",externh_filename);
                            PreVipPrintfExit(error);
                        }
                        n = fprintf(c_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",c_filename);
                            PreVipPrintfExit(error);
                        }
                    }
            
                    else if(nontyped_expansion==TRUE)
                    {
                        TypedExpansion(type_name,temp_file,temp_filename,
                                func_ntype,buffer,type_list);

                        if(strstr(buffer,"break"))
                        {
                            nontyped_expansion = FALSE;

                            fprintf(externh_file,";\n\n");
                            
                            char * files[TYPE_NUMBER];
                            for(i=0;i<func_ntype;i++)
                            {
                                (void)fprintf(temp_file[i],"\n");
                                (void)fclose(temp_file[i]);
                                files[i] = temp_filename[i];
                                if(debug_file==TRUE) printf("fclose 5 %s w\n",temp_filename[i]);

                            }
                            (void)fclose(c_file);
                            filecat( (const char**)&files[0], func_ntype, c_filename, NULL );
                            
                            c_file = fopen(c_filename,"a");
                            if(!c_file)
                            {
                                sprintf(error,"Can not open %s for writing\n",c_filename);
                                PreVipPrintfExit(error);
                            }
                            for(i=0;i<func_ntype;i++)
                            {
                                unlink( temp_filename[i] );
                                            if(debug_file==TRUE) printf("unlink 5 %s \n",temp_filename[i]);
                                temp_file[i] = 0;
                            }
                        }
                    }
                    else if(strstr(buffer,"case"))
                    {
                        if(strstr(buffer,"break"))
                        {
                            PreVipPrintfError("Expansion impossible if case and switch are on the same line!!!");
                            (void)sprintf(error,"(line %d)",count);
                            PreVipPrintfExit(error);
                        }
                        nontyped_expansion = TRUE;
                        TypedExpansion(type_name,temp_file,temp_filename,
                                    func_ntype,buffer,type_list);
                    }
                    else
                    {
                        n = fprintf(c_file,"%s",buffer);
                        if(n!=(int)strlen(buffer))
                        {
                            (void)sprintf(error,"Can not write correctly in %s",c_filename);
                            PreVipPrintfExit(error);
                        }
                    }
                }
                else
                {
                    n = fprintf(c_file,"%s",buffer);
                    if(n!=(int)strlen(buffer))
                    {
                        (void)sprintf(error,"Can not write correctly in %s",c_filename);
                        PreVipPrintfExit(error);
                    }
                }
            }
        }

        if ( !fgets(buffer, BUFFER_MAXLENGTH, gen_file) && !feof(gen_file) )
        {
          sprintf(error,"File %s is corrupted\n",statich_filename);
          PreVipPrintfExit(error);
        }
    }

    if(typed_func_flag==TRUE || nontyped_func_flag==TRUE)
    {
        PreVipPrintfError("Strange end of file...");
    }
    fclose(gen_file);
    fclose(c_file);
    fclose(externh_file);
    fclose(statich_file);

    for(i=0;i<TYPE_NUMBER;i++)
    {
        if( temp_hfile[i] != NULL )
        {
            (void)fclose(temp_hfile[i]);
            unlink( temp_hfilename[i] );
            if(debug_file==TRUE) printf("fclose/unlink %s w\n",temp_hfilename[i]);
            printf( "hfile %s should not exist (end)\n", temp_hfilename[i] );
        }
        if( temp_file[i] != NULL )
        {
            (void)fclose(temp_file[i]);
            unlink( temp_filename[i] );
            if(debug_file==TRUE) printf("fclose/unlink %s w\n",temp_filename[i]);
            printf( "cfile %s should not exist (end)\n", temp_filename[i] );
        }
    }

    return(0);
    }
    
/*----------------------------------------------------------------------------*/
static void TypedExpansion(
char type_name[TYPE_NUMBER][BUFFER_MAXLENGTH],
FILE *type_file[TYPE_NUMBER],
char type_filename[TYPE_NUMBER][BUFFER_MAXLENGTH],
int func_ntype,
char *buffer,
int flag)
/*----------------------------------------------------------------------------*/
{
    int i;
    int n;
    int decalage;
    char typed_buffer[BUFFER_MAXLENGTH];
    char typed_buffer_final[BUFFER_MAXLENGTH];
    char *search;
    int SEARCH;
    char *in, *out;
    char error[BUFFER_MAXLENGTH];

    if(flag!=TRUE)
    {
        PreVipPrintfExit("Lines between TYPED_FUNC_BEGIN and TYPE_LIST");
    }
    for(i=0;i<func_ntype;i++)
    {
        SEARCH = TRUE;
        strcpy(typed_buffer,"");
        search = buffer;
        out = typed_buffer;
        while(SEARCH==TRUE)
        {
            in = search;
            search = strstr(search,"GENTYPE");
            if(search==NULL)
            {
                SEARCH=FALSE;
                while(*in!='\0') *out++ = *in++;
                *out='\0';
            }
            else
            {
                while(in!=search) *out++ = *in++;
                *out='\0';
                strcat(typed_buffer,"Vip_");
                strcat(typed_buffer,type_name[i]);
                out += strlen(type_name[i])+4;
                search += 7;
            }
        }
        SEARCH = TRUE;
        strcpy(typed_buffer_final,"");
        search = typed_buffer;
        out = typed_buffer_final;
        while(SEARCH==TRUE)
        {
            in = search;
            search = strstr(search,"GENSTRING");
            if(search==NULL)
            {
                SEARCH=FALSE;
                while(*in!='\0') *out++ = *in++;
                *out='\0';
            }
            else
            {
                while(in!=search) *out++ = *in++;
                *out='\0';

                /* F. Poupon : a ete rajoute, car les '#define FLOAT' et 
                les '#define DOUBLE' de TivoliIO et Vip_structure cause des 
                problemes avec le compilateur C++ (les classes complex 
                definissent aussi les FLOAT et les DOUBLE) */
                if ((!strcmp(type_name[i], "FLOAT")) || (!strcmp(type_name[i], "DOUBLE")))
            {
                strcat(typed_buffer_final, "V");
                decalage = 1;
            }
                else decalage = 0;
                /*********************************************************/

                strcat(typed_buffer_final,type_name[i]);
                out += strlen(type_name[i])+decalage;
                search += 9;
            }
        }

        n = fprintf(type_file[i],"%s",typed_buffer_final);
        if(n!=(int)strlen(typed_buffer_final))
        {
            sprintf(error,"Can not write correctly in %s",type_filename[i]);
            PreVipPrintfExit(error);
        }
    }
}
/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/
static void         PreVipPrintfExitInFunction( char *fname)
/*----------------------------------------------------------------------------*/
{
  char error[2048];
  (void) sprintf ( error,"Abort expansion of '%s'.", fname );
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n!! VIP Preprocessing Exit" );
  if (strlen(error)!=0)
    (void) fprintf ( stderr, ":\n    %s", error );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
  exit ( EXIT_FAILURE );
}

/*----------------------------------------------------------------------------*/
static void         PreVipPrintfExit( char *fname)
 /*----------------------------------------------------------------------------*/
{
  char error[2048];
		
  (void)sprintf ( error,"Execution aborted: %s.", fname );
  PreVipPrintfError(error);
  exit ( EXIT_FAILURE );
}

/*----------------------------------------------------------------------------*/
static void PreVipPrintfError ( char *s )
/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n!! VIP Preprocessing Error" );
  if (s!=NULL && strlen(s)!=0)
      (void) fprintf ( stderr, ":\n    %s", s );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
}

/*----------------------------------------------------------------------------*/
static void PreVipPrintfWarning ( char *s )
/*----------------------------------------------------------------------------*/
{
  (void) fflush ( stdout );
  (void) fprintf ( stderr, "\n------------------------------" );
  (void) fprintf ( stderr, "\n! VIP Preprocessing Warning" );
  if (s!=NULL && strlen(s)!=0)
      (void) fprintf ( stderr, ":\n    %s", s );
  (void) fprintf( stderr, "\n------------------------------\n" );
  (void) fflush ( stderr );
}
