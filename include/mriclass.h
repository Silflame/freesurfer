#ifndef MRICLASS_H
#define MRICLASS_H

#include "classify.h"
#include "backprop.h"
#include "gclass.h"
#include "artmap.h"
#include "mri.h"
#include "rbf.h"

#define NCLASSES                           5
#define GAUSSIAN_NCLASSES                  NCLASSES
#define BACKGROUND                         0
#define GREY_MATTER                        1
#define GRAY_MATTER                        GREY_MATTER
#define BORDER_MATTER                      2
#define WHITE_MATTER                       3
#define BRIGHT_MATTER                      4

#define LO_LIM                             70
#define HI_LIM                             100
#define DEFINITELY_BACKGROUND              50

/* range for scatter matrix plotting */
#define MAX_SIGMA                          2.0f

#define ISWHITE(c)  (((c) == BORDER_MATTER) || ((c) == WHITE_MATTER))

/* all stuff classified as white below this value is assigned to
   the subcortical gray class, as long as its Talairach coordinate is
   less than TALAIRACH_SUBCORTICAL_GRAY_MAX_Z.
   */
#define HI_SUBCORTICAL_GRAY                85
#define TALAIRACH_SUBCORTICAL_GRAY_MAX_Z   35

#define MAX_INPUTS             40

typedef struct
{
  int  max_clusters[NCLASSES] ;
} RBF_PARMS ;

/* bitfield of feature types */
#define FEATURE_INTENSITY      0x00001
#define FEATURE_ZSCORE3        0x00002
#define FEATURE_ZSCORE5        0x00004
#define FEATURE_DIRECTION      0x00010
#define FEATURE_MEAN3          0x00020
#define FEATURE_MEAN5          0x00040
#define FEATURE_CPOLV_MEAN3    0x00080
#define FEATURE_CPOLV_MEAN5    0x00100
#define FEATURE_CPOLV_MEDIAN3  0x00200
#define FEATURE_CPOLV_MEDIAN5  0x00400
#define FEATURE_MIN3           0x00800
#define FEATURE_MIN5           0x01000
#define FEATURE_MIN7           0x02000
#define FEATURE_X_POSITION     0x04000
#define FEATURE_Y_POSITION     0x08000
#define FEATURE_Z_POSITION     0x10000
#define FEATURE_PRIORS         0x20000
#define FEATURE_POSITION       (FEATURE_X_POSITION | \
                                FEATURE_Y_POSITION | FEATURE_Y_POSITION)

#define FEATURE_CPOLV          (FEATURE_CPOLV_MEAN3 | FEATURE_CPOLV_MEAN5 | \
                                FEATURE_CPOLV_MEDIAN3 | FEATURE_CPOLV_MEDIAN5)
#define MAX_FEATURE            0x80000000

#define MAX_ROUNDS             5

typedef union
{
  BACKPROP   *bp ;
  ARTMAP     *artmap ;
  GCLASSIFY  *gc ;
  RBF        *rbf ;
} CL_UNION ;

typedef struct
{
  int         features[MAX_ROUNDS] ;  /* bit field of above features */
  int         type[MAX_ROUNDS] ;      /* what kind of classifier are we using */
  int         ninputs[MAX_ROUNDS] ;   /* # of inputs to the classifier */
  int         nrounds ;               /* # of times to apply classifier */
  void        *parms ;                /* classifier specific parameters */
  CL_UNION    classifier[MAX_ROUNDS] ;/* pointer to appropriate classifier */
  MRI         *mri_priors ;           /* prior probabilities */
  char        prior_fname[100] ;
} MRI_CLASSIFIER, MRIC ;

MRIC   *MRICalloc(int nrounds, int types[], int features[], void *parms) ;
int    MRICfree(MRIC **pmri) ;
int    MRICtrain(MRIC *mric, char *file_name, char *prior_fname) ;
MRIC   *MRICread(char *fname) ;
MRIC   *MRICquickRead(char *fname) ;
int    MRICwrite(MRIC *mric, char *fname) ;
MRI    *MRICclassify(MRIC *mric, MRI *mri_src, 
                     MRI *mri_dst, float conf,MRI *mri_probs,MRI *mri_classes);
int    MRICcomputeInputs(MRIC *mric, MRI *mri, int x,int y,int z,VECTOR *v_obs,
                         int features);
MRI    *MRICbuildTargetImage(MRI *mri_src, MRI *mri_target, MRI *mri_wm,
                             int lo_lim, int hi_lim) ;
MRI    *MRICupdatePriors(MRI *mri_target, MRI *mri_priors, int scale) ;
int    MRInormalizePriors(MRI *mri_priors) ;
int    MRICupdateStatistics(MRIC *mric, int round, MRI *mri_src, 
                            MRI *mri_wm, MRI_REGION *box) ;
int    MRICcomputeStatistics(MRIC *mric, int round) ;
char   *MRICclassName(MRIC *mric, int round, int classno) ;
int    MRICdump(FILE *fp, MRIC *mric) ;
char   *MRICfeatureName(MRIC *mric, int round, int feature_number) ;
int    MRICfeatureCode(MRIC *mric, int round, int feature_number) ;
int    MRICfeatureNumberCode(int feature_number) ;
int    MRICfeatureNumber(MRIC *mric, int round, int feature_code) ;
int    MRICexamineTrainingSet(MRIC *mric, char *file_name, int round) ;
int    MRICbuildScatterPlot(MRIC *mric, int class, MATRIX *m_scatter,
                 char *training_file_name) ;
int    MRICsetRegionSize(MRIC *mric, int rsize) ;
int    MRICresetRegionSize(MRIC *mric) ;

extern char *class_names[] ;

#endif
