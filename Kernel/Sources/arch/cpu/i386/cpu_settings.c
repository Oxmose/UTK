/*******************************************************************************
 * @file cpu_settings.c
 *
 * @see cpu_settings.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 14/12/2017
 *
 * @version 1.0
 *
 * @brief X86 CPU abstraction functions and definitions.
 *
 * @details X86 CPU abstraction: setting functions and structures, used to set
 * the GDT, IDT and TSS of the CPU. This file also ontains the delarations of
 * the 256 interrupt handlers of the x86 interrupts.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <lib/string.h>       /* String manipulation */
#include <lib/stdint.h>       /* Generic int types */
#include <io/kernel_output.h> /* Kernel output methods */
#include <cpu_structs.h>      /* CPU related structures */

/* UTK configuration file */
#include <config.h>

/* Tests header file */
#if TEST_MODE_ENABLED
#include <Tests/test_bank.h>
#endif

/* Header file */
#include <cpu_settings.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Returns the address of the handler for a given interrupt line.
 *
 * @details Returns the address of the handler attached to the interrupt ID
 * given as parameter.
 *
 * @param[in] int_id This interrupt ID to get the handler of.
 *
 * @return The address of the interrupt handler.
 */
static uintptr_t get_handler(const uint32_t int_id)
{
    switch(int_id)
    {
        case 0:
            return (uintptr_t)interrupt_handler_0;
        case 1:
            return (uintptr_t)interrupt_handler_1;
        case 2:
            return (uintptr_t)interrupt_handler_2;
        case 3:
            return (uintptr_t)interrupt_handler_3;
        case 4:
            return (uintptr_t)interrupt_handler_4;
        case 5:
            return (uintptr_t)interrupt_handler_5;
        case 6:
            return (uintptr_t)interrupt_handler_6;
        case 7:
            return (uintptr_t)interrupt_handler_7;
        case 8:
            return (uintptr_t)interrupt_handler_8;
        case 9:
            return (uintptr_t)interrupt_handler_9;
        case 10:
            return (uintptr_t)interrupt_handler_10;
        case 11:
            return (uintptr_t)interrupt_handler_11;
        case 12:
            return (uintptr_t)interrupt_handler_12;
        case 13:
            return (uintptr_t)interrupt_handler_13;
        case 14:
            return (uintptr_t)interrupt_handler_14;
        case 15:
            return (uintptr_t)interrupt_handler_15;
        case 16:
            return (uintptr_t)interrupt_handler_16;
        case 17:
            return (uintptr_t)interrupt_handler_17;
        case 18:
            return (uintptr_t)interrupt_handler_18;
        case 19:
            return (uintptr_t)interrupt_handler_19;
        case 20:
            return (uintptr_t)interrupt_handler_20;
        case 21:
            return (uintptr_t)interrupt_handler_21;
        case 22:
            return (uintptr_t)interrupt_handler_22;
        case 23:
            return (uintptr_t)interrupt_handler_23;
        case 24:
            return (uintptr_t)interrupt_handler_24;
        case 25:
            return (uintptr_t)interrupt_handler_25;
        case 26:
            return (uintptr_t)interrupt_handler_26;
        case 27:
            return (uintptr_t)interrupt_handler_27;
        case 28:
            return (uintptr_t)interrupt_handler_28;
        case 29:
            return (uintptr_t)interrupt_handler_29;
        case 30:
            return (uintptr_t)interrupt_handler_30;
        case 31:
            return (uintptr_t)interrupt_handler_31;
        case 32:
            return (uintptr_t)interrupt_handler_32;
        case 33:
            return (uintptr_t)interrupt_handler_33;
        case 34:
            return (uintptr_t)interrupt_handler_34;
        case 35:
            return (uintptr_t)interrupt_handler_35;
        case 36:
            return (uintptr_t)interrupt_handler_36;
        case 37:
            return (uintptr_t)interrupt_handler_37;
        case 38:
            return (uintptr_t)interrupt_handler_38;
        case 39:
            return (uintptr_t)interrupt_handler_39;
        case 40:
            return (uintptr_t)interrupt_handler_40;
        case 41:
            return (uintptr_t)interrupt_handler_41;
        case 42:
            return (uintptr_t)interrupt_handler_42;
        case 43:
            return (uintptr_t)interrupt_handler_43;
        case 44:
            return (uintptr_t)interrupt_handler_44;
        case 45:
            return (uintptr_t)interrupt_handler_45;
        case 46:
            return (uintptr_t)interrupt_handler_46;
        case 47:
            return (uintptr_t)interrupt_handler_47;
        case 48:
            return (uintptr_t)interrupt_handler_48;
        case 49:
            return (uintptr_t)interrupt_handler_49;
        case 50:
            return (uintptr_t)interrupt_handler_50;
        case 51:
            return (uintptr_t)interrupt_handler_51;
        case 52:
            return (uintptr_t)interrupt_handler_52;
        case 53:
            return (uintptr_t)interrupt_handler_53;
        case 54:
            return (uintptr_t)interrupt_handler_54;
        case 55:
            return (uintptr_t)interrupt_handler_55;
        case 56:
            return (uintptr_t)interrupt_handler_56;
        case 57:
            return (uintptr_t)interrupt_handler_57;
        case 58:
            return (uintptr_t)interrupt_handler_58;
        case 59:
            return (uintptr_t)interrupt_handler_59;
        case 60:
            return (uintptr_t)interrupt_handler_60;
        case 61:
            return (uintptr_t)interrupt_handler_61;
        case 62:
            return (uintptr_t)interrupt_handler_62;
        case 63:
            return (uintptr_t)interrupt_handler_63;
        case 64:
            return (uintptr_t)interrupt_handler_64;
        case 65:
            return (uintptr_t)interrupt_handler_65;
        case 66:
            return (uintptr_t)interrupt_handler_66;
        case 67:
            return (uintptr_t)interrupt_handler_67;
        case 68:
            return (uintptr_t)interrupt_handler_68;
        case 69:
            return (uintptr_t)interrupt_handler_69;
        case 70:
            return (uintptr_t)interrupt_handler_70;
        case 71:
            return (uintptr_t)interrupt_handler_71;
        case 72:
            return (uintptr_t)interrupt_handler_72;
        case 73:
            return (uintptr_t)interrupt_handler_73;
        case 74:
            return (uintptr_t)interrupt_handler_74;
        case 75:
            return (uintptr_t)interrupt_handler_75;
        case 76:
            return (uintptr_t)interrupt_handler_76;
        case 77:
            return (uintptr_t)interrupt_handler_77;
        case 78:
            return (uintptr_t)interrupt_handler_78;
        case 79:
            return (uintptr_t)interrupt_handler_79;
        case 80:
            return (uintptr_t)interrupt_handler_80;
        case 81:
            return (uintptr_t)interrupt_handler_81;
        case 82:
            return (uintptr_t)interrupt_handler_82;
        case 83:
            return (uintptr_t)interrupt_handler_83;
        case 84:
            return (uintptr_t)interrupt_handler_84;
        case 85:
            return (uintptr_t)interrupt_handler_85;
        case 86:
            return (uintptr_t)interrupt_handler_86;
        case 87:
            return (uintptr_t)interrupt_handler_87;
        case 88:
            return (uintptr_t)interrupt_handler_88;
        case 89:
            return (uintptr_t)interrupt_handler_89;
        case 90:
            return (uintptr_t)interrupt_handler_90;
        case 91:
            return (uintptr_t)interrupt_handler_91;
        case 92:
            return (uintptr_t)interrupt_handler_92;
        case 93:
            return (uintptr_t)interrupt_handler_93;
        case 94:
            return (uintptr_t)interrupt_handler_94;
        case 95:
            return (uintptr_t)interrupt_handler_95;
        case 96:
            return (uintptr_t)interrupt_handler_96;
        case 97:
            return (uintptr_t)interrupt_handler_97;
        case 98:
            return (uintptr_t)interrupt_handler_98;
        case 99:
            return (uintptr_t)interrupt_handler_99;
        case 100:
            return (uintptr_t)interrupt_handler_100;
        case 101:
            return (uintptr_t)interrupt_handler_101;
        case 102:
            return (uintptr_t)interrupt_handler_102;
        case 103:
            return (uintptr_t)interrupt_handler_103;
        case 104:
            return (uintptr_t)interrupt_handler_104;
        case 105:
            return (uintptr_t)interrupt_handler_105;
        case 106:
            return (uintptr_t)interrupt_handler_106;
        case 107:
            return (uintptr_t)interrupt_handler_107;
        case 108:
            return (uintptr_t)interrupt_handler_108;
        case 109:
            return (uintptr_t)interrupt_handler_109;
        case 110:
            return (uintptr_t)interrupt_handler_110;
        case 111:
            return (uintptr_t)interrupt_handler_111;
        case 112:
            return (uintptr_t)interrupt_handler_112;
        case 113:
            return (uintptr_t)interrupt_handler_113;
        case 114:
            return (uintptr_t)interrupt_handler_114;
        case 115:
            return (uintptr_t)interrupt_handler_115;
        case 116:
            return (uintptr_t)interrupt_handler_116;
        case 117:
            return (uintptr_t)interrupt_handler_117;
        case 118:
            return (uintptr_t)interrupt_handler_118;
        case 119:
            return (uintptr_t)interrupt_handler_119;
        case 120:
            return (uintptr_t)interrupt_handler_120;
        case 121:
            return (uintptr_t)interrupt_handler_121;
        case 122:
            return (uintptr_t)interrupt_handler_122;
        case 123:
            return (uintptr_t)interrupt_handler_123;
        case 124:
            return (uintptr_t)interrupt_handler_124;
        case 125:
            return (uintptr_t)interrupt_handler_125;
        case 126:
            return (uintptr_t)interrupt_handler_126;
        case 127:
            return (uintptr_t)interrupt_handler_127;
        case 128:
            return (uintptr_t)interrupt_handler_128;
        case 129:
            return (uintptr_t)interrupt_handler_129;
        case 130:
            return (uintptr_t)interrupt_handler_130;
        case 131:
            return (uintptr_t)interrupt_handler_131;
        case 132:
            return (uintptr_t)interrupt_handler_132;
        case 133:
            return (uintptr_t)interrupt_handler_133;
        case 134:
            return (uintptr_t)interrupt_handler_134;
        case 135:
            return (uintptr_t)interrupt_handler_135;
        case 136:
            return (uintptr_t)interrupt_handler_136;
        case 137:
            return (uintptr_t)interrupt_handler_137;
        case 138:
            return (uintptr_t)interrupt_handler_138;
        case 139:
            return (uintptr_t)interrupt_handler_139;
        case 140:
            return (uintptr_t)interrupt_handler_140;
        case 141:
            return (uintptr_t)interrupt_handler_141;
        case 142:
            return (uintptr_t)interrupt_handler_142;
        case 143:
            return (uintptr_t)interrupt_handler_143;
        case 144:
            return (uintptr_t)interrupt_handler_144;
        case 145:
            return (uintptr_t)interrupt_handler_145;
        case 146:
            return (uintptr_t)interrupt_handler_146;
        case 147:
            return (uintptr_t)interrupt_handler_147;
        case 148:
            return (uintptr_t)interrupt_handler_148;
        case 149:
            return (uintptr_t)interrupt_handler_149;
        case 150:
            return (uintptr_t)interrupt_handler_150;
        case 151:
            return (uintptr_t)interrupt_handler_151;
        case 152:
            return (uintptr_t)interrupt_handler_152;
        case 153:
            return (uintptr_t)interrupt_handler_153;
        case 154:
            return (uintptr_t)interrupt_handler_154;
        case 155:
            return (uintptr_t)interrupt_handler_155;
        case 156:
            return (uintptr_t)interrupt_handler_156;
        case 157:
            return (uintptr_t)interrupt_handler_157;
        case 158:
            return (uintptr_t)interrupt_handler_158;
        case 159:
            return (uintptr_t)interrupt_handler_159;
        case 160:
            return (uintptr_t)interrupt_handler_160;
        case 161:
            return (uintptr_t)interrupt_handler_161;
        case 162:
            return (uintptr_t)interrupt_handler_162;
        case 163:
            return (uintptr_t)interrupt_handler_163;
        case 164:
            return (uintptr_t)interrupt_handler_164;
        case 165:
            return (uintptr_t)interrupt_handler_165;
        case 166:
            return (uintptr_t)interrupt_handler_166;
        case 167:
            return (uintptr_t)interrupt_handler_167;
        case 168:
            return (uintptr_t)interrupt_handler_168;
        case 169:
            return (uintptr_t)interrupt_handler_169;
        case 170:
            return (uintptr_t)interrupt_handler_170;
        case 171:
            return (uintptr_t)interrupt_handler_171;
        case 172:
            return (uintptr_t)interrupt_handler_172;
        case 173:
            return (uintptr_t)interrupt_handler_173;
        case 174:
            return (uintptr_t)interrupt_handler_174;
        case 175:
            return (uintptr_t)interrupt_handler_175;
        case 176:
            return (uintptr_t)interrupt_handler_176;
        case 177:
            return (uintptr_t)interrupt_handler_177;
        case 178:
            return (uintptr_t)interrupt_handler_178;
        case 179:
            return (uintptr_t)interrupt_handler_179;
        case 180:
            return (uintptr_t)interrupt_handler_180;
        case 181:
            return (uintptr_t)interrupt_handler_181;
        case 182:
            return (uintptr_t)interrupt_handler_182;
        case 183:
            return (uintptr_t)interrupt_handler_183;
        case 184:
            return (uintptr_t)interrupt_handler_184;
        case 185:
            return (uintptr_t)interrupt_handler_185;
        case 186:
            return (uintptr_t)interrupt_handler_186;
        case 187:
            return (uintptr_t)interrupt_handler_187;
        case 188:
            return (uintptr_t)interrupt_handler_188;
        case 189:
            return (uintptr_t)interrupt_handler_189;
        case 190:
            return (uintptr_t)interrupt_handler_190;
        case 191:
            return (uintptr_t)interrupt_handler_191;
        case 192:
            return (uintptr_t)interrupt_handler_192;
        case 193:
            return (uintptr_t)interrupt_handler_193;
        case 194:
            return (uintptr_t)interrupt_handler_194;
        case 195:
            return (uintptr_t)interrupt_handler_195;
        case 196:
            return (uintptr_t)interrupt_handler_196;
        case 197:
            return (uintptr_t)interrupt_handler_197;
        case 198:
            return (uintptr_t)interrupt_handler_198;
        case 199:
            return (uintptr_t)interrupt_handler_199;
        case 200:
            return (uintptr_t)interrupt_handler_200;
        case 201:
            return (uintptr_t)interrupt_handler_201;
        case 202:
            return (uintptr_t)interrupt_handler_202;
        case 203:
            return (uintptr_t)interrupt_handler_203;
        case 204:
            return (uintptr_t)interrupt_handler_204;
        case 205:
            return (uintptr_t)interrupt_handler_205;
        case 206:
            return (uintptr_t)interrupt_handler_206;
        case 207:
            return (uintptr_t)interrupt_handler_207;
        case 208:
            return (uintptr_t)interrupt_handler_208;
        case 209:
            return (uintptr_t)interrupt_handler_209;
        case 210:
            return (uintptr_t)interrupt_handler_210;
        case 211:
            return (uintptr_t)interrupt_handler_211;
        case 212:
            return (uintptr_t)interrupt_handler_212;
        case 213:
            return (uintptr_t)interrupt_handler_213;
        case 214:
            return (uintptr_t)interrupt_handler_214;
        case 215:
            return (uintptr_t)interrupt_handler_215;
        case 216:
            return (uintptr_t)interrupt_handler_216;
        case 217:
            return (uintptr_t)interrupt_handler_217;
        case 218:
            return (uintptr_t)interrupt_handler_218;
        case 219:
            return (uintptr_t)interrupt_handler_219;
        case 220:
            return (uintptr_t)interrupt_handler_220;
        case 221:
            return (uintptr_t)interrupt_handler_221;
        case 222:
            return (uintptr_t)interrupt_handler_222;
        case 223:
            return (uintptr_t)interrupt_handler_223;
        case 224:
            return (uintptr_t)interrupt_handler_224;
        case 225:
            return (uintptr_t)interrupt_handler_225;
        case 226:
            return (uintptr_t)interrupt_handler_226;
        case 227:
            return (uintptr_t)interrupt_handler_227;
        case 228:
            return (uintptr_t)interrupt_handler_228;
        case 229:
            return (uintptr_t)interrupt_handler_229;
        case 230:
            return (uintptr_t)interrupt_handler_230;
        case 231:
            return (uintptr_t)interrupt_handler_231;
        case 232:
            return (uintptr_t)interrupt_handler_232;
        case 233:
            return (uintptr_t)interrupt_handler_233;
        case 234:
            return (uintptr_t)interrupt_handler_234;
        case 235:
            return (uintptr_t)interrupt_handler_235;
        case 236:
            return (uintptr_t)interrupt_handler_236;
        case 237:
            return (uintptr_t)interrupt_handler_237;
        case 238:
            return (uintptr_t)interrupt_handler_238;
        case 239:
            return (uintptr_t)interrupt_handler_239;
        case 240:
            return (uintptr_t)interrupt_handler_240;
        case 241:
            return (uintptr_t)interrupt_handler_241;
        case 242:
            return (uintptr_t)interrupt_handler_242;
        case 243:
            return (uintptr_t)interrupt_handler_243;
        case 244:
            return (uintptr_t)interrupt_handler_244;
        case 245:
            return (uintptr_t)interrupt_handler_245;
        case 246:
            return (uintptr_t)interrupt_handler_246;
        case 247:
            return (uintptr_t)interrupt_handler_247;
        case 248:
            return (uintptr_t)interrupt_handler_248;
        case 249:
            return (uintptr_t)interrupt_handler_249;
        case 250:
            return (uintptr_t)interrupt_handler_250;
        case 251:
            return (uintptr_t)interrupt_handler_251;
        case 252:
            return (uintptr_t)interrupt_handler_252;
        case 253:
            return (uintptr_t)interrupt_handler_253;
        case 254:
            return (uintptr_t)interrupt_handler_254;
        case 255:
            return (uintptr_t)interrupt_handler_255;
        default:
            return (uintptr_t)interrupt_handler_0;

    }
}

/**
 * @brief Formats a GDT entry.
 *
 * @details Formats data given as parameter into a standard GDT entry.
 * The result is directly written in the memory pointed by the entry parameter.
 *
 * @param[out] entry The pointer to the entry structure to format.
 * @param[in] base  The base address of the segment for the GDT entry.
 * @param[in] limit The limit address of the segment for the GDT entry.
 * @param[in] type  The type of segment for the GDT entry.
 * @param[in] flags The flags to be set for the GDT entry.
 */
static void format_gdt_entry(uint64_t* entry,
                             const uint32_t base, const uint32_t limit,
                             const unsigned char type, const uint32_t flags)
{
    uint32_t lo_part = 0;
    uint32_t hi_part = 0;

    /*
     * Low part[31;0] = Base[15;0] Limit[15;0]
     */
    lo_part = ((base & 0xFFFF) << 16) | (limit & 0xFFFF);

    /*
     * High part[7;0] = Base[23;16]
     */
    hi_part = (base >> 16) & 0xFF;
    /*
     * High part[11;8] = Type[3;0]
     */
    hi_part |= (type & 0xF) << 8;
    /*
     * High part[15;12] = Seg_Present[1;0]Privilege[2;0]Descriptor_Type[1;0]
     * High part[23;20] = Granularity[1;0]Op_Size[1;0]L[1;0]AVL[1;0]
     */
    hi_part |= flags & 0x00F0F000;

    /*
     * High part[19;16] = Limit[19;16]
     */
    hi_part |= limit & 0xF0000;
    /*
     * High part[31;24] = Base[31;24]
     */
    hi_part |= base & 0xFF000000;

    /* Set the value of the entry */
    *entry = lo_part | (((uint64_t) hi_part) << 32);
}

/**
 * @brief Formats an IDT entry.
 *
 * @details Formats data given as parameter into a standard IDT entry.
 * The result is directly written in the memory pointed by the entry parameter.
 *
 * @param[out] entry The pointer to the entry structure to format.
 * @param[in] handler The handler function for the IDT entry.
 * @param[in] type  The type of segment for the IDT entry.
 * @param[in] flags The flags to be set for the IDT entry.
 */
static void format_idt_entry(uint64_t* entry,
                             const uint32_t handler,
                             const unsigned char type, const uint32_t flags)
{
    uint32_t lo_part = 0;
    uint32_t hi_part = 0;

    /*
     * Low part[31;0] = Selector[15;0] Handler[15;0]
     */
    lo_part = (KERNEL_CS_32 << 16) | (handler & 0x0000FFFF);

    /*
     * High part[7;0] = Handler[31;16] Flags[4;0] Type[4;0] ZERO[7;0]
     */
    hi_part = (handler & 0xFFFF0000) |
              ((flags & 0xF0) << 8) | ((type & 0x0F) << 8);

    /* Set the value of the entry */
    *entry = lo_part | (((uint64_t) hi_part) << 32);
}

void cpu_setup_gdt(void)
{
    uint32_t i;

#if CPU_KERNEL_DEBUG == 1
    kernel_serial_debug("[CPU] Setting CPU GDT\n");
#endif
    /************************************
     * KERNEL GDT ENTRIES
     ***********************************/

    /* Set the kernel code descriptor */
    uint32_t kernel_code_seg_flags = GDT_FLAG_GRANULARITY_4K |
                                     GDT_FLAG_32_BIT_SEGMENT |
                                     GDT_FLAG_PL0 |
                                     GDT_FLAG_SEGMENT_PRESENT |
                                     GDT_FLAG_CODE_TYPE;

    uint32_t kernel_code_seg_type =  GDT_TYPE_EXECUTABLE |
                                     GDT_TYPE_READABLE |
                                     GDT_TYPE_PROTECTED;

    /* Set the kernel data descriptor */
    uint32_t kernel_data_seg_flags = GDT_FLAG_GRANULARITY_4K |
                                     GDT_FLAG_32_BIT_SEGMENT |
                                     GDT_FLAG_PL0 |
                                     GDT_FLAG_SEGMENT_PRESENT |
                                     GDT_FLAG_DATA_TYPE;

    uint32_t kernel_data_seg_type =  GDT_TYPE_WRITABLE |
                                     GDT_TYPE_GROW_DOWN;

    /* Set the kernel 16 bits code descriptor */
    uint32_t kernel_code_16_seg_flags = GDT_FLAG_GRANULARITY_4K |
                                        GDT_FLAG_16_BIT_SEGMENT |
                                        GDT_FLAG_PL0 |
                                        GDT_FLAG_SEGMENT_PRESENT |
                                        GDT_FLAG_CODE_TYPE;

    uint32_t kernel_code_16_seg_type =  GDT_TYPE_EXECUTABLE |
                                        GDT_TYPE_READABLE |
                                        GDT_TYPE_PROTECTED;

    /* Set the kernel 16 bits data descriptor */
    uint32_t kernel_data_16_seg_flags = GDT_FLAG_GRANULARITY_4K |
                                        GDT_FLAG_16_BIT_SEGMENT |
                                        GDT_FLAG_PL0 |
                                        GDT_FLAG_SEGMENT_PRESENT |
                                        GDT_FLAG_DATA_TYPE;

    uint32_t kernel_data_16_seg_type =  GDT_TYPE_WRITABLE |
                                        GDT_TYPE_GROW_DOWN;

    /************************************
     * TSS ENTRY
     ***********************************/

    uint32_t tss_seg_flags = GDT_FLAG_32_BIT_SEGMENT |
                             GDT_FLAG_SEGMENT_PRESENT |
                             GDT_FLAG_PL0;

    uint32_t tss_seg_type = GDT_TYPE_ACCESSED |
                            GDT_TYPE_EXECUTABLE;

    /* Blank the GDT, set the NULL descriptor */
    memset(cpu_gdt, 0, sizeof(uint64_t) * GDT_ENTRY_COUNT);

    /* Load the segments */
    format_gdt_entry(&cpu_gdt[KERNEL_CS_32 / 8],
                     KERNEL_CODE_SEGMENT_BASE_32, KERNEL_CODE_SEGMENT_LIMIT_32,
                     kernel_code_seg_type, kernel_code_seg_flags);

    format_gdt_entry(&cpu_gdt[KERNEL_DS_32 / 8],
                     KERNEL_DATA_SEGMENT_BASE_32, KERNEL_DATA_SEGMENT_LIMIT_32,
                     kernel_data_seg_type, kernel_data_seg_flags);

    format_gdt_entry(&cpu_gdt[KERNEL_CS_16 / 8],
                     KERNEL_CODE_SEGMENT_BASE_16, KERNEL_CODE_SEGMENT_LIMIT_16,
                     kernel_code_16_seg_type, kernel_code_16_seg_flags);

    format_gdt_entry(&cpu_gdt[KERNEL_DS_16 / 8],
                     KERNEL_DATA_SEGMENT_BASE_16, KERNEL_DATA_SEGMENT_LIMIT_16,
                     kernel_data_16_seg_type, kernel_data_16_seg_flags);

    format_gdt_entry(&cpu_gdt[TSS_SEGMENT / 8],
                     (uintptr_t)&cpu_tss,
                     ((uintptr_t)(&cpu_tss)) + sizeof(cpu_tss_entry_t),
                     tss_seg_type, tss_seg_flags);

    for(i = 1; i < MAX_CPU_COUNT; ++i)
    {
        format_gdt_entry(&cpu_gdt[(TSS_SEGMENT + i * 0x08) / 8],
                         (uintptr_t)&cpu_tss[i - 1],
                         ((uintptr_t)(&cpu_tss[i - 1])) +
                            sizeof(cpu_tss_entry_t),
                         tss_seg_type, tss_seg_flags);
    }

    /* Set the GDT descriptor */
    cpu_gdt_ptr.size = ((sizeof(uint64_t) * GDT_ENTRY_COUNT) - 1);
    cpu_gdt_ptr.base = (uintptr_t)&cpu_gdt;

    /* Load the GDT */
    __asm__ __volatile__("lgdt %0" :: "m" (cpu_gdt_ptr.size), "m" (cpu_gdt_ptr.base));

    /* Load segment selectors with a far jump for CS*/
    __asm__ __volatile__("movw %w0,%%ds\n\t"
                         "movw %w0,%%es\n\t"
                         "movw %w0,%%fs\n\t"
                         "movw %w0,%%gs\n\t"
                         "movw %w0,%%ss\n\t" :: "r" (KERNEL_DS_32));
    __asm__ __volatile__("ljmp %0, $new_gdt \n\t new_gdt: \n\t" :: "i" (KERNEL_CS_32));

    kernel_success("GDT Initialized at 0x%p\n", cpu_gdt_ptr.base);

#if TEST_MODE_ENABLED
    gdt_test();
#endif
}

void cpu_setup_idt(void)
{
    uint32_t i;

#if CPU_KERNEL_DEBUG == 1
    kernel_serial_debug("[CPU] Setting CPU IDT\n");
#endif

    /* Blank the IDT */
    memset(cpu_idt, 0, sizeof(uint64_t) * IDT_ENTRY_COUNT);

    /* Set interrupt handlers for each interrupt
     * This allows to redirect all interrupts to a global handler in C
     */
    for(i = 0; i < IDT_ENTRY_COUNT; ++i)
    {
        format_idt_entry(&cpu_idt[i],
                         get_handler(i),
                         IDT_TYPE_INT_GATE, IDT_FLAG_PRESENT | IDT_FLAG_PL0);
    }

    /* Set the GDT descriptor */
    cpu_idt_ptr.size = ((sizeof(uint64_t) * IDT_ENTRY_COUNT) - 1);
    cpu_idt_ptr.base = (uintptr_t)&cpu_idt;

    /* Load the GDT */
    __asm__ __volatile__("lidt %0" :: "m" (cpu_idt_ptr.size), "m" (cpu_idt_ptr.base));

    kernel_success("IDT Initialized at 0x%p\n", cpu_idt_ptr.base);

#if TEST_MODE_ENABLED
    idt_test();
#endif
}

void cpu_setup_tss(void)
{
    int32_t i;
#if CPU_KERNEL_DEBUG == 1
    kernel_serial_debug("[CPU] Setting CPU TSS\n");
#endif

    /* Blank the TSS */
    memset(cpu_tss, 0, sizeof(cpu_tss_entry_t) * MAX_CPU_COUNT);

    /* Set basic values */
    for(i = 0; i < MAX_CPU_COUNT - 1; ++i)
    {
        cpu_tss[i].ss0 = KERNEL_DS_32;
        cpu_tss[i].esp0 = (uintptr_t)(cpu_stacks[i] + KERNEL_STACK_SIZE);

        cpu_tss[i].es = KERNEL_DS_32;
        cpu_tss[i].cs = KERNEL_CS_32;
        cpu_tss[i].ss = KERNEL_DS_32;
        cpu_tss[i].ds = KERNEL_DS_32;
        cpu_tss[i].fs = KERNEL_DS_32;
        cpu_tss[i].gs = KERNEL_DS_32;

        cpu_tss[i].iomap_base = sizeof(cpu_tss_entry_t);
    }

    /* Load TSS */
    __asm__ __volatile__("ltr %0" : : "rm" ((uint16_t)(TSS_SEGMENT)));

    kernel_success("TSS Initialized at 0x%p\n", cpu_tss);

#if TEST_MODE_ENABLED
    tss_test();
#endif
}