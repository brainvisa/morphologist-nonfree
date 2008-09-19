/****************************************************************************
 * PROJECT     : V.I.P. Library
 * MODULE NAME : Vip_random.c         * TYPE     : Function
 * AUTHOR      : MANGIN J.-F.         * CREATION : 22/01/1997
 * VERSION     : 0.1                  * REVISION :
 * LANGUAGE    : C++                  * EXAMPLE  :
 * DEVICE      : Sun SPARC Station 5
 ****************************************************************************
 *
 * DESCRIPTION : Generation de nombre aleatoires
 *
 ****************************************************************************
 *
 * USED MODULES : Vip_alloc.h - Vip_math.h
 *
 ****************************************************************************
 * REVISIONS :  DATE  |    AUTHOR    |       DESCRIPTION
 *--------------------|--------------|---------------------------------------
 *              / /   |              |
 ****************************************************************************/

#include <vip/alloc.h>
#include <vip/math.h>

void VipFreeRandomBox(RandomBox *rb_list)
{
    RandomBox *killer=NULL, *new=NULL;

    new = rb_list;
    killer = rb_list->next;
    new->next = NULL;

    while (killer != NULL)	
	{
	    rb_list = killer->next;
	    VipFree(killer);
	    killer = rb_list;
	}
}

RandomBox *VipCreateRandomBox(unsigned char label, float proba)
{ 
    RandomBox *new=NULL;
    char message[256];

    if ((proba < 0.0) || (proba > 1.0))
	{
	    sprintf(message, "Bad proba: %f\n", proba);
	    VipPrintfError(message);
	    VipPrintfExit("(Vip_random)VipCreateRandomBox");
	    return((RandomBox *)NULL);
	}

    new = (RandomBox *)VipCalloc(1, sizeof(RandomBox), NULL);
    if (!new)
	{
	    VipPrintfError("Error while allocating new RandomBox entry");
	    VipPrintfExit("(Vip_random)VipCreateRandomBox");
	    return((RandomBox *)NULL);
	}

    new->label = label;
    new->proba = proba;
    new->next = NULL;

    return(new);
}

RandomBox *VipCreateRandomBoxCircularList(int size, int nb_label)
{
    RandomBox *rbox_list=NULL, *tail=NULL, *new=NULL;
    int i=0;
    unsigned char label=0;
    float proba=0.0;

    if (size < 0)
	{
	    VipPrintfError("Not a coherent size");
	    VipPrintfExit("(Vip_random)VipCreateRandomBoxCircularList");
	    return((RandomBox *)NULL);
	}

    label = (unsigned char)((rand()) % nb_label);
    proba = (float)(rand())/(float)RAND_MAX;
    rbox_list = VipCreateRandomBox(label, proba);
    if (!rbox_list)
	{
	    VipPrintfError("Unable to create the first RandomBox entry");
	    VipPrintfExit("(Vip_random)VipCreateRandomBoxCircularList");
	    return((RandomBox *)NULL);
	}

    tail = rbox_list; 
 
    for (i=size; i--;)
	{
	    label = (unsigned char)((rand()) % nb_label);
	    proba = (float)(rand())/(float)RAND_MAX;
	    new = VipCreateRandomBox(label, proba);
	    if (!new)
		{
		    VipPrintfError("Unable to create the RandomBox entries");
		    VipPrintfExit("(random)VipCreateRandomBoxCircularList");
		    return((RandomBox *)NULL);
		}

	    tail->next = new;
	    tail = new;
	}

    tail->next = rbox_list;

    return(rbox_list);	
} 
