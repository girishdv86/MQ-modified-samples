/* @(#) samples/c/amqssuba.c, lqm, p701, p701-101-091221 1.15.1.1 09/08/16 14:54:32 */
 /********************************************************************/
 /*                                                                  */
 /* Program name: AMQSSUBA                                           */
 /*                                                                  */
 /* Description: Sample C program that subscribes and gets messages  */
 /*              from a topic (example using MQSUB). A managed       */
 /*              destination queue is used.                          */
 /* <N_OCO_COPYRIGHT>                                                */
 /* Licensed Materials - Property of IBM                             */
 /*                                                                  */
 /* 5724-H72                                                         */
 /* (c) Copyright IBM Corp. 1994, 2008 All Rights Reserved.          */
 /*                                                                  */
 /* US Government Users Restricted Rights - Use, duplication or      */
 /* disclosure restricted by GSA ADP Schedule Contract with          */
 /* IBM Corp.                                                        */
 /* <NOC_COPYRIGHT>                                                  */
 /********************************************************************/
 /*                                                                  */
 /* Function:                                                        */
 /*                                                                  */
 /*                                                                  */
 /*   AMQSSUBA is a sample C program to subscribe and get messages   */
 /*   from a topic.  It is an example of MQSUB.                      */
 /*                                                                  */
 /*   sample                                                         */
 /*      -- subscribes non-durably to the topic named in the 1st     */
 /*         parameter                                                */
 /*                                                                  */
 /*      -- calls MQGET repeatedly to get messages from the topic,   */
 /*         and writes to stdout, assuming each message to represent */
 /*         a line of text to be written                             */
 /*                                                                  */
 /*      -- writes a message for each MQI reason other than          */
 /*         MQRC_NONE; stops if there is a MQI completion code       */
 /*         of MQCC_FAILED                                           */
 /*                                                                  */
 /*                                                                  */
 /*   Program logic:                                                 */
 /*      Take name of input topic from the 1st parameter             */
 /*      MQSUB topic for INPUT                                       */
 /*      while no MQI failures,                                      */
 /*      .  MQGET next message                                       */
 /*      .  print the result                                         */
 /*      .  (no message available counts as failure, and loop ends)  */
 /*      MQCLOSE the topic                                           */
 /*                                                                  */
 /*                                                                  */
 /********************************************************************/
 /*                                                                  */
 /*   AMQSSUBA has the following parameters                          */
 /*       required:                                                  */
 /*                 (1) The name of the topic                        */
 /*       optional:                                                  */
 /*                 (2) Queue manager name                           */
 /*                 (3) The MQSD.Options to pass into MQSUB          */
 /*                                                                  */
 /********************************************************************/
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
     /* includes for MQI  */
 #include <cmqc.h>

 int main(int argc, char **argv)
 {

   /*   Declare MQI structures needed                                */
   MQSD     sd = {MQSD_DEFAULT};    /* Subscription Descriptor       */
   MQMD     md = {MQMD_DEFAULT};    /* Message Descriptor            */
   MQGMO   gmo = {MQGMO_DEFAULT};   /* get message options           */
      /** note, sample uses defaults where it can **/

   MQHCONN  Hcon;                   /* connection handle             */
   MQHOBJ   Hobj = MQHO_NONE;       /* object handle used for MQGET  */
   MQHOBJ   Hsub = MQHO_NONE;       /* object handle                 */
   MQLONG   C_options;              /* MQCLOSE options               */
   MQLONG   CompCode;               /* completion code               */
   MQLONG   S_CompCode;             /* MQSUB completion code         */
   MQLONG   Reason;                 /* reason code                   */
   MQLONG   CReason;                /* reason code for MQCONN        */
   MQBYTE   buffer[101];            /* message buffer                */
   MQLONG   buflen;                 /* buffer length                 */
   MQLONG   messlen;                /* message length received       */
   char     QMName[50];             /* queue manager name            */
   MQLONG   O_options;
   MQOD     od = {MQOD_DEFAULT};
   MQLONG   OpenCode; 

   printf("Sample AMQSSUBA start\n");
   if (argc < 2)
   {
     printf("Required parameter missing - topic string\n");
     exit(99);
   }

   QMName[0] = 0;
   if (argc > 2)
     strncpy(QMName, argv[2], sizeof(QMName));

   /******************************************************************/
   /*                                                                */
   /*   Connect to queue manager                                     */
   /*                                                                */
   /******************************************************************/
   MQCONN(QMName,                  /* queue manager                  */
          &Hcon,                   /* connection handle              */
          &CompCode,               /* completion code                */
          &CReason);               /* reason code                    */

   /* report reason and stop if it failed     */
   if (CompCode == MQCC_FAILED)
   {
     printf("MQCONN ended with reason code %d\n", CReason);
     exit( (int)CReason );
   }

   /******************************************************************/
   /*                                                                */
   /*   Subscribe using a managed destination queue                  */
   /*                                                                */
   /******************************************************************/

   sd.Options =   MQSO_CREATE
                | MQSO_NON_DURABLE
                | MQSO_FAIL_IF_QUIESCING;
/*                | MQSO_MANAGED; */

   strcpy(od.ObjectName, "SUBQ");
     O_options = MQOO_OUTPUT|MQOO_INPUT_AS_Q_DEF            /* open queue for input      */
               | MQOO_FAIL_IF_QUIESCING; /* but not if MQM stopping   */
   MQOPEN(Hcon,                      /* connection handle            */
          &od,                       /* object descriptor for queue  */
          O_options,                 /* open options                 */
          &Hobj,                     /* object handle                */
          &OpenCode,                 /* completion code              */
          &Reason);                  /* reason code                  */
   if (Reason != MQRC_NONE)
   {
     printf("MQOPEN ended with reason code %d\n", Reason);
   }

   if (OpenCode == MQCC_FAILED)
   {
     printf("unable to open queue for input\n");
   }

   if (argc > 3)
   {
     sd.Options = atoi( argv[3] );
     printf("MQSUB SD.Options are %d\n", sd.Options);
   }

   sd.ObjectString.VSPtr = argv[1];
   sd.ObjectString.VSLength = (MQLONG)strlen(argv[1]);

   MQSUB(Hcon,                       /* connection handle            */
         &sd,                        /* object descriptor for queue  */
         &Hobj,                      /* object handle (output)       */
         &Hsub,                      /* object handle (output)       */
         &S_CompCode,                /* completion code              */
         &Reason);                   /* reason code                  */

   /* report reason, if any; stop if failed      */
   if (Reason != MQRC_NONE)
   {
     printf("MQSUB ended with reason code %d\n", Reason);
   }

   if (S_CompCode == MQCC_FAILED)
   {
     printf("unable to subscribe to topic\n");
   }

   /******************************************************************/
   /*                                                                */
   /*   Get messages from the destination queue                      */
   /*   Loop until there is a failure                                */
   /*                                                                */
   /******************************************************************/
   CompCode = S_CompCode;       /* use MQOPEN result for initial test  */

   /******************************************************************/
   /* Use these options when connecting to Queue Managers that also  */
   /* support them, see the Application Programming Reference for    */
   /* details.                                                       */
   /* These options cause the MsgId and CorrelId to be replaced, so  */
   /* that there is no need to reset them before each MQGET          */
   /******************************************************************/
   gmo.Options =   MQGMO_WAIT         /* wait for new messages       */
                 | MQGMO_NO_SYNCPOINT /* no transaction              */
                 | MQGMO_CONVERT;     /* convert if necessary        */

   gmo.WaitInterval = 30000;        /* 30 second limit for waiting   */

   while (CompCode != MQCC_FAILED)
   {
     buflen = sizeof(buffer) - 1; /* buffer size available for GET   */

     /****************************************************************/
     /* The following two statements are not required if the MQGMO   */
     /* version is set to MQGMO_VERSION_2 and and gmo.MatchOptions   */
     /* is set to MQGMO_NONE                                         */
     /****************************************************************/
     /*                                                              */
     /*   In order to read the messages in sequence, MsgId and       */
     /*   CorrelID must have the default value.  MQGET sets them     */
     /*   to the values in for message it returns, so re-initialise  */
     /*   them before every call                                     */
     /*                                                              */
     /****************************************************************/
     memcpy(md.MsgId, MQMI_NONE, sizeof(md.MsgId));
     memcpy(md.CorrelId, MQCI_NONE, sizeof(md.CorrelId));

     /****************************************************************/
     /*                                                              */
     /*   MQGET sets Encoding and CodedCharSetId to the values in    */
     /*   the message returned, so these fields should be reset to   */
     /*   the default values before every call, as MQGMO_CONVERT is  */
     /*   specified.                                                 */
     /*                                                              */
     /****************************************************************/

     md.Encoding       = MQENC_NATIVE;
     md.CodedCharSetId = MQCCSI_Q_MGR;
     printf("Calling MQGET : %d seconds wait time\n",
            gmo.WaitInterval / 1000);

     MQGET(Hcon,                /* connection handle                 */
           Hobj,                /* object handle                     */
           &md,                 /* message descriptor                */
           &gmo,                /* get message options               */
           buflen,              /* buffer length                     */
           buffer,              /* message buffer                    */
           &messlen,            /* message length                    */
           &CompCode,           /* completion code                   */
           &Reason);            /* reason code                       */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       if (Reason == MQRC_NO_MSG_AVAILABLE)
       {                         /* special report for normal end    */
         printf("no more messages\n");
       }
       else                      /* general report for other reasons */
       {
         printf("MQGET ended with reason code %d\n", Reason);

         /*   treat truncated message as a failure for this sample   */
         if (Reason == MQRC_TRUNCATED_MSG_FAILED)
         {
           CompCode = MQCC_FAILED;
         }
       }
     }

     /****************************************************************/
     /*   Display each message received                              */
     /****************************************************************/
     if (CompCode != MQCC_FAILED)
     {
       buffer[messlen] = '\0';            /* add terminator          */
       printf("message <%s>\n", buffer);
     }
   }

   /******************************************************************/
   /*                                                                */
   /*   Close the subscription handle                                */
   /*                                                                */
   /******************************************************************/
   if (S_CompCode != MQCC_FAILED)
   {
     C_options = MQCO_NONE;        /* no close options             */

     MQCLOSE(Hcon,                    /* connection handle           */
             &Hsub,                   /* object handle               */
             C_options,
             &CompCode,               /* completion code             */
             &Reason);                /* reason code                 */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       printf("MQCLOSE ended with reason code %d\n", Reason);
     }
   }
   /******************************************************************/
   /*                                                                */
   /*   Close the managed destination queue (if it was opened)       */
   /*                                                                */
   /******************************************************************/
   if (S_CompCode != MQCC_FAILED)
   {
     C_options = MQCO_NONE;

     MQCLOSE(Hcon,                    /* connection handle           */
             &Hobj,                   /* object handle               */
             C_options,
             &CompCode,               /* completion code             */
             &Reason);                /* reason code                 */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       printf("MQCLOSE ended with reason code %d\n", Reason);
     }
   }

   /******************************************************************/
   /*                                                                */
   /*   Disconnect from MQM if not already connected                 */
   /*                                                                */
   /******************************************************************/
   if (CReason != MQRC_ALREADY_CONNECTED )
   {
     MQDISC(&Hcon,                     /* connection handle          */
            &CompCode,                 /* completion code            */
            &Reason);                  /* reason code                */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       printf("MQDISC ended with reason code %d\n", Reason);
     }
   }

   /******************************************************************/
   /*                                                                */
   /* END OF AMQSSUBA                                                */
   /*                                                                */
   /******************************************************************/
   printf("Sample AMQSSUBA end\n");
   return(0);
 }
