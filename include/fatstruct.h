#pragma once
#include <QDateTime>

enum class FatType
{
	FAT12,
	FAT16,
	FAT32,
	UNKNOWN
};

namespace FatConstants
{
	constexpr quint8 ATTR_READ_ONLY = 0x01;
	constexpr quint8 ATTR_HIDDEN = 0x02;
	constexpr quint8 ATTR_SYSTEM = 0x04;
	constexpr quint8 ATTR_VOLUME_ID = 0x08;
	constexpr quint8 ATTR_DIRECTORY = 0x10;
	constexpr quint8 ATTR_ARCHIVE = 0x20;
	constexpr quint8 ATTR_LONG_NAME = 0x0F;

	constexpr quint8 DIR_ENTRY_END = 0x00;
	constexpr quint8 DIR_ENTRY_DELETED = 0xE5;

	constexpr quint32 FAT16_EOF_MIN = 0xFFF8;
	constexpr quint32 FAT32_EOF_MIN = 0x0FFFFFF8;
	constexpr quint32 FAT16_MASK = 0xFFFF;
	constexpr quint32 FAT32_MASK = 0x0FFFFFFF;

	constexpr quint32 FAT12_MAX_CLUSTERS = 4085;
	constexpr quint32 FAT16_MAX_CLUSTERS = 65525;

	constexpr int DIRECTORY_ENTRY_SIZE = 32;
	constexpr int BPB_SIZE = 512;
	constexpr int SHORT_NAME_LENGTH = 11;
	constexpr int VOLUME_LABEL_LENGTH = 11;

	constexpr quint8 JUMP_SHORT = 0xEB;
	constexpr quint8 JUMP_SHORT_NOP = 0x90;
	constexpr quint8 JUMP_NEAR = 0xE9;

	constexpr quint16 LFN_FILL_CHAR = 0xFFFF;
	constexpr quint16 LFN_NULL_CHAR = 0x0000;

	constexpr quint8 MEDIA_TYPES[] = { 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

	constexpr quint16 VALID_SECTOR_SIZES[] = { 512, 1024, 2048, 4096 };
}	 // namespace FatConstants

#pragma pack(push, 1)	 // disable padding

struct ExtendedBpbFat16
{
	quint8 BS_DrvNum;
	quint8 BS_Reserved1;
	quint8 BS_BootSig;
	quint32 BS_VolID;
	quint8 BS_VolLab[11];
	quint8 BS_FilSysType[8];
	quint8 reserved[448];
	quint8 Signature_word[2];
};

struct ExtendedBpbFat32
{
	quint32 BPB_FATSz32;
	quint16 BPB_ExtFlags;
	quint8 BPB_FSVer[2];
	quint32 BPB_RootClus;
	quint16 BPB_FSInfo;
	quint16 BPB_BkBootSec;
	quint8 BPB_Reserved[12];
	quint8 BS_DrvNum;
	quint8 BS_Reserved1;
	quint8 BS_BootSig;
	quint32 BS_VolID;
	quint8 BS_VolLab[11];
	quint8 BS_FilSysType[8];
	quint8 reserved[420];
	quint8 Signature_word[2];
};

struct BiosParameterBlock
{
	quint8 BS_jmpBoot[3];
	quint8 BS_OEMName[8];
	quint16 BPB_BytsPerSec;
	quint8 BPB_SecPerClus;
	quint16 BPB_RsvdSecCnt;
	quint8 BPB_NumFATs;
	quint16 BPB_RootEntCnt;
	quint16 BPB_TotSec16;
	quint8 BPB_Media;
	quint16 BPB_FATSz16;
	quint16 BPB_SecPerTrk;
	quint16 BPB_NumHeads;
	quint32 BPB_HiddSec;
	quint32 BPB_TotSec32;

	union
	{
		ExtendedBpbFat16 fat16;
		ExtendedBpbFat32 fat32;
	};

	// remaining bytes
};

#pragma pack(pop)

#pragma pack(push, 1)	 // disable padding

struct DirectoryEntry
{
	quint8 DIR_Name[11];
	quint8 DIR_Attr;
	quint8 DIR_NTRes;
	quint8 DIR_CrtTimeTenth;
	quint16 DIR_CrtTime;
	quint16 DIR_CrtDate;
	quint16 DIR_LstAccDate;
	quint16 DIR_FstClusHI;
	quint16 DIR_WrtTime;
	quint16 DIR_WrtDate;
	quint16 DIR_FstClusLO;
	quint32 DIR_FileSize;
};

#pragma pack(pop)

#pragma pack(push, 1)
struct LongDirectoryEntry
{
	quint8 LDIR_Ord;
	quint16 LDIR_Name1[5];
	quint8 LDIR_Attr;
	quint8 LDIR_Type;
	quint8 LDIR_Chksum;
	quint16 LDIR_Name2[6];
	quint16 LDIR_FstClusLO;
	quint16 LDIR_Name3[2];
};
#pragma pack(pop)
