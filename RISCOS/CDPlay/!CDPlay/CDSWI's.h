/******************************************************************************

Program:        CDPlayer

File:           CDBitz.c

Function:       CDPlayer Wimp C Program

Description:    CD Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 23rd Febuary 1996
                                                                             
******************************************************************************/

#define CD_Version                       0x41240
#define CD_ReadData                      0x41241
#define CD_SeekTo                        0x41242
#define CD_DriveStatus                   0x41243
#define CD_DriveReady                    0x41244
#define CD_GetParameters                 0x41245
#define CD_SetParameters                 0x41246
#define CD_OpenDrawer                    0x41247
#define CD_EjectButton                   0x41248
#define CD_EnquireAddress                0x41249
#define CD_EnquireDataMode               0x4124A
#define CD_PlayAudio                     0x4124B
#define CD_PlayTrack                     0x4124C
#define CD_AudioPause                    0x4124D
#define CD_EnquireTrack                  0x4124E
#define CD_ReadSubChannel                0x4124F
#define CD_CheckDrive                    0x41250
#define CD_DiscChanged                   0x41251
#define CD_StopDisc                      0x41252
#define CD_DiscUsed                      0x41253
#define CD_AudioStatus                   0x41254
#define CD_Inquiry                       0x41255
#define CD_DiscHasChanged                0x41256
#define CD_Control                       0x41257
#define CD_Supported                     0x41258
#define CD_Prefetch                      0x41259
#define CD_Reset                         0x4125A
#define CD_CloseDrawer                   0x4125B
#define CD_IsDrawerLocked                0x4125C
#define CD_AudioControl                  0x4125D
#define CD_LastError                     0x4125E
#define CD_AudioLevel                    0x4125F
#define CD_Register                      0x41260
#define CD_Unregister                    0x41261
#define CD_ByteCopy                      0x41262
#define CD_Identify                      0x41263
#define CD_ConvertToLBA                  0x41264
#define CD_ConvertToMSF                  0x41265
#define CD_ReadAudio                     0x41266
#define CDFS_ConvertDriveToDevice        0x41E80
#define CDFS_SetBufferSize               0x41E81
#define CDFS_GetBufferSize               0x41E82
#define CDFS_SetNumberOfDrives           0x41E83
#define CDFS_GetNumberOfDrives           0x41E84
#define CDFS_GiveFileType                0x41E85
#define CDFS_DescribeDisc                0x41E86
#define CDFS_WhereIsFile                 0x41E87
#define CDFS_Truncation                  0x41E88
#define XCD_Version                      0x61240
#define XCD_ReadData                     0x61241
#define XCD_SeekTo                       0x61242
#define XCD_DriveStatus                  0x61243
#define XCD_DriveReady                   0x61244
#define XCD_GetParameters                0x61245
#define XCD_SetParameters                0x61246
#define XCD_OpenDrawer                   0x61247
#define XCD_EjectButton                  0x61248
#define XCD_EnquireAddress               0x61249
#define XCD_EnquireDataMode              0x6124A
#define XCD_PlayAudio                    0x6124B
#define XCD_PlayTrack                    0x6124C
#define XCD_AudioPause                   0x6124D
#define XCD_EnquireTrack                 0x6124E
#define XCD_ReadSubChannel               0x6124F
#define XCD_CheckDrive                   0x61250
#define XCD_DiscChanged                  0x61251
#define XCD_StopDisc                     0x61252
#define XCD_DiscUsed                     0x61253
#define XCD_AudioStatus                  0x61254
#define XCD_Inquiry                      0x61255
#define XCD_DiscHasChanged               0x61256
#define XCD_Control                      0x61257
#define XCD_Supported                    0x61258
#define XCD_Prefetch                     0x61259
#define XCD_Reset                        0x6125A
#define XCD_CloseDrawer                  0x6125B
#define XCD_IsDrawerLocked               0x6125C
#define XCD_AudioControl                 0x6125D
#define XCD_LastError                    0x6125E
#define XCD_AudioLevel                   0x6125F
#define XCD_Register                     0x61260
#define XCD_Unregister                   0x61261
#define XCD_ByteCopy                     0x61262
#define XCD_Identify                     0x61263
#define XCD_ConvertToLBA                 0x61264
#define XCD_ConvertToMSF                 0x61265
#define XCD_ReadAudio                    0x61266
#define XCD_GetAudioParameters		 0x61269
#define XCD_SetAudioParameters		 0x6126A
#define XCDFS_ConvertDriveToDevice       0x61E80
#define XCDFS_SetBufferSize              0x61E81
#define XCDFS_GetBufferSize              0x61E82
#define XCDFS_SetNumberOfDrives          0x61E83
#define XCDFS_GetNumberOfDrives          0x61E84
#define XCDFS_GiveFileType               0x61E85
#define XCDFS_DescribeDisc               0x61E86
#define XCDFS_WhereIsFile                0x61E87
#define XCDFS_Truncation                 0x61E88

