
#ifndef analyze_db_HEADER
#define analyze_db_HEADER 1

/* SCCS control */
/*static char SCCSIDH_analyze_db[] = "@(#)analyze_db.h	1.2\t7/30/90";*/



   struct hdr_key
    {
      int size;
      char data_type[10];
      char name[18];
      int extents;
      short int session_error;
      char regular;
      char hkey_un0;
     };

  struct image_dimension
   {
      short int dim[16];
      short int bitpix;
      short int dim_un0;
      float pixdim[16];
      int glmax,glmin;
   };

  struct hdr_history 
   {
         char descrip[80];
         char aux_file[24];
         char orient;
         char originator[10];
         char generated[10];
         char scannum[10];
         char patient_id[10];
         char exp_date[10];
         char exp_time[10];
         char hist_un0[3];
         int views;
         int vols_added;
         int start_field;
         int field_skip;
         int omax,omin;
         int smax,smin;
   };

  struct analyze_db 
    {
       struct hdr_key          key;
       struct image_dimension  img;
       struct hdr_history      his;
     };



#endif

