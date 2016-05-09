
/*
 * Copyright (c) 2012 Karl N. Redgate
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/** \file BIOS.cc
 * \brief 
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <errno.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <syslog.h>
#include <tcl.h>

#include "smbios.h"

namespace {
    typedef SMBIOS::Structure *(*StructureFactory)( void * );
    static const int MAX_FACTORY = 256;
    static StructureFactory factories[MAX_FACTORY];
    static const char *unknown = "UNKNOWN";
    int debug = 0;

    /**
     */
    void
    print_rdf_header( FILE *f ) {
        fprintf( f, "<?xml version='1.0' encoding='UTF-8'?>\n" );
        fprintf( f, "<rdf:RDF " );
        fprintf( f, "xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#' " );
        fprintf( f, "xmlns:rdfs='http://www.w3.org/2000/01/rdf-schema#' " );
        fprintf( f, "xmlns:dc='http://purl.org/dc/elements/1.1/' " );
        fprintf( f, "xmlns:dcterms='http://purl.org/dc/terms/' " );
        fprintf( f, "xmlns:smbios='http://redgates.com/smbios/1.0/'>" );
    }

    /**
     */
    void
    print_rdf_trailer( FILE *f ) {
        fprintf( f, "</rdf:RDF>" );
    }

    /**
     */
    void
    print_field_rdf( FILE *f, const char *element, const char *value ) {
        fprintf( f, "<smbios:%s>%s</smbios:%s>", element, value, element );
    }

    /**
     */
    void
    print_field_rdf( FILE *f, const char *element, uint8_t value ) {
        fprintf( f, "<smbios:%s>%d</smbios:%s>", element, value, element );
    }
}

namespace SMBIOS {

    /**
     */
    class StringList {
    private:
        int _count;
        const char **strings;
        const char *_end;
        void parse( const char *, int );
    public:
        StringList( char * );
        ~StringList();
        const int count() const { return _count; }
        const char *end() const { return _end; }
        const char * operator [] ( int index ) const { return strings[index]; }
    };

}

/**
 */
SMBIOS::StringList::StringList( char *data ) : strings(0) {
    parse( data, 0 );
}

/**
 */
SMBIOS::StringList::~StringList() {
    if ( strings != 0 )  free( strings );
}

/**
 * start with n=0, increment with each recursion
 * at end of recursion set the string pointer
 */
void SMBIOS::StringList::parse( const char *data, int n ) {
    if ( *data == '\0' ) { // end of strings
        _count = n;
        if ( _count != 0 ) {
            strings = (const char **)malloc( sizeof(char*) * n );
            _end = data + 1;
        } else {
            strings = 0;
            _end = data + 2;
        }
        return;
    }
    const char *p = data;
    while ( *p != '\0' ) p++;
    parse( p+1, n+1 );
    strings[n] = data;
}

/*
 * system enclosure or chassis type names
 */
namespace {
    const char * const SystemTypeName[] = {
        "Unspecified",
        "Other",
        "Unknown",
        "Desktop",
        "Low Profile Desktop",
        "Pizza Box",
        "Mini Tower",
        "Tower",
        "Portable",
        "Laptop",
        "Notebook",
        "Hand Held",
        "Docking Station",
        "All In One",
        "Sub Notebook",
        "Space-saving",
        "Lunch Box",
        "Main Server Chassis", /* master.mif says System */
        "Expansion Chassis",
        "Sub Chassis",
        "Bus Expansion Chassis",
        "Peripheral Chassis",
        "RAID Chassis", 
        "Rack Mount Chassis",   
        "Sealed-case PC",
        "Multi-system", /* 0x19 */
        "Compact PCI",
        "Advanced TCA",
        "Blade",
        "Blade Enclosure"
    };
}

/**
 * There may not be anythign necessary here
 */
SMBIOS::Structure::Structure( void *address ) {
    data = (uint8_t *)address;
    structure_type = data[0];
    header_length = data[1];
    handle = word_at( 2 );
    strings = new StringList( (char *)(data + header_length) );
    StringList &s = *strings;

    int count = strings->count();
    if ( debug ) printf( "%d strings\n", count );

    for ( int i = 0 ; i < count ; i++ ) {
        if ( debug ) printf( "S[%d]: '%s'\n", i, s[i] );
    }
}

/**
 */
SMBIOS::Structure::~Structure() {
    delete strings;
}

/**
 */
const void *
SMBIOS::Structure::next() const {
    return strings->end();
}

/**
 * There may not be anythign necessary here
 */
const char *
SMBIOS::Structure::string( uint8_t n ) const {
    uint8_t index = data[n] - 1;
    StringList &s = *strings;
    if ( s.count() < index ) return NULL;
    return s[index];
}

/**
 */
uint16_t
SMBIOS::Structure::word_at( int offset ) const {
    uint16_t value;
    value  = data[offset+1] << 8;
    value += data[offset];
    return value;
}

/**
 */
void
SMBIOS::Structure::print_xml( FILE *f ) {
    fprintf( f, "<smbios:%s>", structure_name() );
    fprintf( f, "<smbios:structure-handle rdf:value='0x%04x' />", handle );
    fprintf( f, "<smbios:structure-type rdf:value='%d' />", structure_type );
    print_fields( f );
    fprintf( f, "</smbios:%s>", structure_name() );

#if 0
    fprintf( f, "<structure handle='0x%04x' type='%d'>\n", handle, structure_type );
    fprintf( f, "<name>%s</name>", structure_name() );
    fprintf( f, "<fields>\n" );
    print_fields( f );
    fprintf( f, "</fields>\n" );
    print_strings( f );
    fprintf( f, "</structure>\n" );
#endif
}

/**
 */
void
SMBIOS::Structure::print_strings( FILE *f ) {
    StringList &list = *strings;
    int count = list.count();
    if ( count == 0 ) return;

    fprintf( f, "<strings count='%d'>\n", count );
    for ( int i = 0 ; i < count ; i++ ) {
        const char *s = list[i];
        if ( s == NULL ) {
            fprintf( f, "<s id='%d'/>\n", i );
            continue;
        }
        fprintf( f, "<s id='%d'>%s</s>\n", i, s );
    }
    fprintf( f, "</strings>\n" );
}

/**
 */
void
SMBIOS::Structure::print_fields( FILE *f ) {
}

/**
 */
void
SMBIOS::Structure::print_field( FILE *f, const char *element, const char *value ) {
    print_field_rdf( f, element, value );
}

/**
 */
void
SMBIOS::Structure::print_field( FILE *f, const char *element, uint8_t value ) {
    print_field_rdf( f, element, value );
}

/**
 */
SMBIOS::Inactive::Inactive( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed Inactive\n" );
}

/**
 * Don't print any fields for "inactive" structures.
 */
void
SMBIOS::Inactive::print_fields( FILE *f ) {
}

/**
 * read data in address and constuct -- and set string list
 */
SMBIOS::BIOSInformation::BIOSInformation( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed BIOSInformation\n" );
    _vendor = string(0x4);
    _version = string(0x5);
    _release_date = string(0x8);
}

/**
 */
void
SMBIOS::BIOSInformation::print_fields( FILE *f ) {
    print_field( f, "vendor", _vendor );
    print_field( f, "version", _version );
    print_field( f, "release-date", _release_date );
}

/**
 */
SMBIOS::System::System( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed System\n" );
}

/**
 */
const char * SMBIOS::System::manufacturer()  const { return string(0x4); } // version 2.0
const char * SMBIOS::System::product_name()  const { return string(0x5); } // version 2.0
const char * SMBIOS::System::serial_number() const { return string(0x7); } // version 2.0
const char * SMBIOS::System::sku_number()    const { return string(0x19); } // Version 2.4
const char * SMBIOS::System::family()        const { return string(0x1A); } // Version 2.4

/**
 */
uint8_t *
SMBIOS::System::uuid_raw() const {
    return data + 8;
}

/**
 */
GUID *
SMBIOS::System::guid_raw() const {
    return (GUID *)( data + 8 );
}

/**
 */
UUID *
SMBIOS::System::uuid() {
    return new UUID( guid_raw() );
}

/**
 */
uint8_t
SMBIOS::System::type_id() const {
    return data[0x5] & 0x7F;
}

namespace {

    const char * const WakeUpTypeName[] = {
        "Reserved",
        "Other",
        "Unknown",
        "APM Timer",
        "Modem Ring",
        "LAN Remote",
        "Power Switch",
        "PCI PME#",
        "AC Power Restored"
    };

}

/**
 */
const char *
SMBIOS::System::wake_up_type() const {
    uint8_t index = data[0x18];

    if ( index > 0x8 ) {
        return "OUT OF SPEC";
    }

    return WakeUpTypeName[index];
}


/**
 */
void
SMBIOS::System::print_fields( FILE *f ) {
    print_field( f, "manufacturer", manufacturer() );
    print_field( f, "type", SystemTypeName[type_id()] );
    print_field( f, "version", string(0x6) );
    print_field( f, "serial-number", serial_number() );
    print_field( f, "sku-number", sku_number() );
    print_field( f, "family", family() );
    print_field( f, "asset-tag", string(0x8) );
    print_field( f, "product-name", product_name() );
    print_field( f, "wake-up-type", wake_up_type() );

    uint8_t height = data[0x11];
    if ( height != 0 )  print_field( f, "height", height );

    uint8_t power_cords = data[0x12];
    if ( power_cords != 0 )  print_field( f, "power-cords", power_cords );
}

/**
 */
SMBIOS::BaseBoard::BaseBoard( SMBIOS::System *system )
: SMBIOS::Structure( system->address() ) {
}

/**
 */
SMBIOS::BaseBoard::BaseBoard( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed BaseBoard\n" );
}

/**
 */
const char *  SMBIOS::BaseBoard::manufacturer()     const { return string(0x4); }
const char *  SMBIOS::BaseBoard::product_name()     const { return string(0x5); }
const char *  SMBIOS::BaseBoard::version()          const { return string(0x6); }
const char *  SMBIOS::BaseBoard::serial_number()    const { return string(0x7); }
const char *  SMBIOS::BaseBoard::asset_tag()        const { return string(0x8); }
const char *  SMBIOS::BaseBoard::chassis_location() const { return string(0xA); }
uint8_t SMBIOS::BaseBoard::features()         const { return data[0x9]; }
uint8_t SMBIOS::BaseBoard::type_id()          const { return data[0xD]; }

namespace {

    const char * const BaseBoardType[] = {
        "OUT OF SPEC",
        "Unknown",
        "Other",
        "Server Blade",
        "Connectivity Switch",
        "System Management Module",
        "Processor Module",
        "I/O Module",
        "Memory Module",
        "Daughter board",
        "Motherboard (includes processor, memory, and I/O)",
        "Processor/Memory Module",
        "Processor/IO Module",
        "Interconnect board"
    };

}

/**
 */
const char *
SMBIOS::BaseBoard::board_type() const {
    uint8_t index = data[0xD];

    if ( index > 0xD ) {
        return "OUT OF SPEC";
    }

    return BaseBoardType[index];
}

/**
 */
void
SMBIOS::BaseBoard::print_fields( FILE *f ) {
    print_field( f, "manufacturer", manufacturer() );
    print_field( f, "product-name", product_name() );
    print_field( f, "serial-number", serial_number() );
    print_field( f, "version", version() );
    print_field( f, "asset-tag", asset_tag() );
    print_field( f, "chassis-location", chassis_location() );
    print_field( f, "board-type", board_type() );
}

/**
 */
SMBIOS::Chassis::Chassis( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed Chassis\n" );
}

/**
 */
const char *
SMBIOS::Chassis::manufacturer() const {
    return (header_length < 0x9) ? unknown : string(0x4);
}

/**
 */
const char *
SMBIOS::Chassis::version() const {
    return (header_length < 0x9) ? unknown : string(0x6);
}

/**
 */
const char *
SMBIOS::Chassis::serial_number() const {
    return (header_length < 0x9) ? unknown : string(0x7);
}

/**
 */
const char *
SMBIOS::Chassis::asset_tag() const {
    return (header_length < 0x9) ? unknown : string(0x8);
}

/**
 */
uint8_t SMBIOS::Chassis::chassis_type_id() const {
    return (header_length < 0x9) ? 0 : (data[0x5] & 0x7F);
}

/**
 */
const char *
SMBIOS::Chassis::chassis_name() const {
    return SystemTypeName[chassis_type_id()];
}

/**
 */
uint8_t SMBIOS::Chassis::chassis_lock_id() const {
    return (header_length < 0x9) ? 0 : (data[0x5] >> 7);
}

/**
 */
uint8_t SMBIOS::Chassis::bootup_state_id() const {
    return (header_length < 0xD) ? 0 : data[0x9];
}

/**
 */
uint8_t SMBIOS::Chassis::power_supply_state_id() const {
    return (header_length < 0xD) ? 0 : data[0xA];
}

/**
 */
uint8_t SMBIOS::Chassis::thermal_state_id() const {
    return (header_length < 0xD) ? 0 : data[0xB];
}

/**
 */
uint8_t SMBIOS::Chassis::security_state_id() const {
    return (header_length < 0xD) ? 0 : data[0xC];
}

/**
 */
uint8_t SMBIOS::Chassis::power_cords() const {
    return (header_length < 0x15) ? 0 : data[0x12];
}

/**
 */
void
SMBIOS::Chassis::print_fields( FILE *f ) {
    print_field( f, "manufacturer", manufacturer() );
    print_field( f, "serial-number", serial_number() );
    print_field( f, "version", version() );
    print_field( f, "asset-tag", asset_tag() );
}

/**
 */
SMBIOS::Processor::Processor( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed Processor\n" );
}

/**
 */
const char *
SMBIOS::Processor::socket_designation() const {
    if ( header_length < 0x1A ) return unknown;
    return string(0x4);
}

/**
 */
const char *
SMBIOS::Processor::manufacturer() const {
    if ( header_length < 0x1A ) return unknown;
    return string(0x7);
}

/**
 */
const char *
SMBIOS::Processor::version() const {
    if ( header_length < 0x1A ) return unknown;
    return string(0x10);
}

/**
 */
const char *
SMBIOS::Processor::serial_number() const {
    if ( header_length < 0x23 ) return unknown;
    return string(0x20);
}

/**
 */
const char *
SMBIOS::Processor::asset_tag() const {
    if ( header_length < 0x23 ) return unknown;
    return string(0x21);
}

/**
 */
const char *
SMBIOS::Processor::part_number() const {
    if ( header_length < 0x23 ) return unknown;
    return string(0x22);
}

/**
 */
bool SMBIOS::Processor::is_populated()     const { return ( data[0x18] & (1<<6) ) != 0; }
bool SMBIOS::Processor::is_enabled()       const { return ( data[0x18] & 0x7 ) == 1; }
bool SMBIOS::Processor::is_user_disabled() const { return ( data[0x18] & 0x7 ) == 2; }
bool SMBIOS::Processor::is_bios_disabled() const { return ( data[0x18] & 0x7 ) == 3; }
bool SMBIOS::Processor::is_idle()          const { return ( data[0x18] & 0x7 ) == 4; }

/**
 */
void
SMBIOS::Processor::print_fields( FILE *f ) {
    print_field( f, "socket-designation", socket_designation() );
    print_field( f, "manufacturer", manufacturer() );
    print_field( f, "version", version() );
    print_field( f, "serial-number", serial_number() );
    print_field( f, "asset-tag", asset_tag() );
    print_field( f, "part-number", part_number() );
}

/**
 */
SMBIOS::Cache::Cache( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed Cache\n" );
}

/*
 */
namespace {
    const char * const SystemCacheTypeName[] = {
        "Unspecified",
        "Other",
        "Unknown",
        "Instruction",
        "Data",
        "Unified"
    };
}

/**
 */
const char *
SMBIOS::Cache::system_cache_type() const {
    uint8_t index = data[0x11];

    if ( index > 0x5 ) {
        return "OUT OF SPEC";
    }

    return SystemCacheTypeName[index];
}

/**
 */
void
SMBIOS::Cache::print_fields( FILE *f ) {
    print_field( f, "system-cache-type", system_cache_type() );
}

/**
 */
SMBIOS::PortConnector::PortConnector( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed PortConnector\n" );
}

const char * SMBIOS::PortConnector::internal_designator()  const { return string(0x4); }
const char * SMBIOS::PortConnector::external_designator()  const { return string(0x6); }

/**
 */
void
SMBIOS::PortConnector::print_fields( FILE *f ) {
    print_field( f, "internal-reference-designator", internal_designator() );
    print_field( f, "external-reference-designator", external_designator() );
}

/**
 */
SMBIOS::SystemSlot::SystemSlot( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed SystemSlot\n" );
}

const char * SMBIOS::SystemSlot::designation()  const { return string(0x4); }

/**
 */
void
SMBIOS::SystemSlot::print_fields( FILE *f ) {
    print_field( f, "designation", designation() );
}

/**
 * type 10
 */
SMBIOS::OnboardDevices::OnboardDevices( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed OnboardDevices\n" );
}

/**
 */
void
SMBIOS::OnboardDevices::print_fields( FILE *f ) {
}

/**
 * type 11
 */
SMBIOS::OEMStrings::OEMStrings( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed OEMStrings\n" );
}

/**
 */
void
SMBIOS::OEMStrings::print_fields( FILE *f ) {
}

/**
 * type 16
 */
SMBIOS::PhysicalMemoryArray::PhysicalMemoryArray( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed PhysicalMemoryArray\n" );
}

namespace {

    const char * const MemoryDeviceFormFactorName[] = {
        "Unspecified",
        "Other",
        "Unknown",
        "SIMM",
        "SIP",
        "Chip",
        "DIP",
        "ZIP",
        "Proprietary Card",
        "DIMM",
        "TSOP",
        "Row of chips",
        "RIMM",
        "SODIMM",
        "SRIMM",
        "FB-DIMM",
        "Multi-system" /* 0x19 */
    };

    const char * const MemoryDeviceTypeName[] = {
        "Unspecified",
        "Other",
        "Unknown",
        "DRAM",
        "EDRAM",
        "VRAM",
        "SRAM",
        "RAM",
        "ROM",
        "FLASH",
        "EEPROM",
        "FEPROM",
        "EPROM",
        "CDRAM",
        "3DRAM",
        "SDRAM",
        "SGRAM",
        "RDRAM",
        "DDR",
        "DDR2",
        "DDR2 FB-DIMM",
        "Reserved",
        "Reserved",
        "Reserved",
        "DDR3",
        "FBD2" /* 0x19 */
    };

}

/**
 * type 17
 */
SMBIOS::MemoryDevice::MemoryDevice( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed MemoryDevice\n" );
    if ( debug ) printf( " total_width = %d bits\n", total_width() );
    if ( debug ) printf( " data_width  = %d bits\n", data_width() );
    if ( debug ) printf( " form_factor = %s\n", form_factor() );
    if ( debug ) printf( " memory_type = %s\n", memory_type() );
}

/**
 */
uint16_t SMBIOS::MemoryDevice::total_width() const {
    return word_at(0x08);
}

/**
 */
uint16_t SMBIOS::MemoryDevice::data_width() const {
    return word_at(0x0A);
}

/**
 */
const char *
SMBIOS::MemoryDevice::form_factor() const {
    int index = data[0x0E];
    return MemoryDeviceFormFactorName[ index ];
}

/**
 */
const char *
SMBIOS::MemoryDevice::memory_type() const {
    int index = data[0x12];
    return MemoryDeviceTypeName[ index ];
}

/**
 */
void
SMBIOS::MemoryDevice::print_fields( FILE *f ) {
    print_field( f, "total-width", total_width() );
    print_field( f, "data-width", data_width() );
    print_field( f, "form-factor", form_factor() );
    print_field( f, "memory-type", memory_type() );
}

/**
 * type 32
 */
SMBIOS::SystemBoot::SystemBoot( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed SystemBoot\n" );
}

/**
 * type 41
 */
SMBIOS::OnboardDevicesExtended::OnboardDevicesExtended( void *address )
: SMBIOS::Structure(address) {
    if ( debug ) printf( "constructed OnboardDevicesExtended\n" );
}


/**
 * read each table recursively?
 *
 * do not need to recurse, allocate Structures based on count passed in
 * then probe each one calling the correct constructor method based on the
 * structure type.
 *
 * each structure should probe its own string list and use that to pass back
 * the address of the next Structure in the memory image
 */
SMBIOS::Table::Table( uint8_t *base, uint16_t length, uint16_t count )
: _count(count), bios(0), system(0), baseboard(0), chassis(0) {
    if ( debug ) printf( "DMI is at %p length %hd count %hd\n", base, length, count );

    int fd = open( "/dev/mem", O_RDONLY);
    if ( fd == -1 ) {
        perror( "cannot open mem" );
        exit( 1 );
    }

    dmi = (Structure **)malloc( sizeof(Structure *) * count );
    size_t offset = (size_t)base % getpagesize();

    uint8_t *mapped_address = (uint8_t *)mmap( 0, offset + length, PROT_READ, MAP_SHARED, fd, (off_t)base - offset );
    if ( mapped_address == MAP_FAILED ) {
        perror( "cannot map dmi table address" );
        exit( 1 );
    }
    address = (void *)( mapped_address + offset );
    if ( debug ) printf( "mapped dmi table\n" );
    close( fd );

    // struct dmi_header *h = (struct dmi_header *)address;
    uint8_t *p = (uint8_t*)address;
    for ( int i = 0 ; i < count ; i++ ) {
        uint8_t type = p[0];
        uint8_t len = p[1];

        if ( debug ) printf( "DMI header %d type %d len %d -- ", i, type, len );
        StructureFactory factory = factories[type];
        SMBIOS::Structure *structure = factory( p );
        dmi[i] = structure;

        switch (type) {
        case 0: bios = structure; break;
        case 1: system = structure; break;
        case 2: baseboard = structure; break;
        case 3: chassis = structure; break;
        case 4:
            sockets.push_back( (SMBIOS::Processor*)structure );
            break;
        }

        p = (uint8_t *)( structure->next() );
    }

    if ( baseboard == NULL ) {
        syslog( LOG_WARNING, "BaseBoard information missing from SMBIOS tables" );
        baseboard = new SMBIOS::BaseBoard( system );
    }
}

/**
 */
SMBIOS::Table::~Table() {
    if ( dmi == NULL ) return;
    for ( int i = 0 ; i < _count ; i++ ) {
        if ( dmi[i] == NULL ) continue;
        delete dmi[i];
    }
    free( dmi );
}

/**
 */
void
SMBIOS::Table::print_xml( FILE *f ) {
    if ( dmi == NULL ) return;
    for ( int i = 0 ; i < _count ; i++ ) {
        if ( dmi[i] == NULL ) continue;
        dmi[i]->print_xml( f );
    }
}

/**
 */
SMBIOS::Header::Header()
: table(0), dmi_address(0), dmi_length(0) { }

/**
 */
SMBIOS::Header::~Header() {
    if ( table != 0 ) delete table;
}

/**
 */
void SMBIOS::Header::probe( uint8_t *data ) {
    if ( data[0x10] == '_' && data[0x11] == 'D' && data[0x12] == 'M' && data[0x13] == 'I' && data[0x14] == '_' ) {
        if ( debug > 3 ) printf( "I see DMI header\n" );
    } else {
        if ( debug > 3 ) printf( "I DO NOT see DMI header\n" );
    }
    major_version = data[0x6];
    minor_version = data[0x7];
    syslog( LOG_NOTICE, "SMBIOS version %u.%u", major_version, minor_version );
    if ( debug ) printf( "SMBIOS version %u.%u\n", major_version, minor_version );

#pragma GCC diagnostic push ignored "-Wint-to-pointer-cast"
    uintptr_t address = *( (uint32_t *)(data + 0x18) );
    dmi_address = (uint8_t *) address;
#pragma GCC diagnostic pop
    dmi_length  = *( (uint16_t*) (data + 0x16) );
    dmi_number  = *( (uint16_t*) (data + 0x1C) );
    dmi_version = (data[0x6] << 8) + data[0x7];
    table = new Table( dmi_address, dmi_length, dmi_number );
}

/** Factory method
 * 
 */
uint8_t *
SMBIOS::Header::locate() {
    for ( int i = 0 ; i < MAX_FACTORY ; i++ ) {
        factories[i] = SMBIOS::Structure::Factory;
    }
    // \todo -- get rid of this memset... because of ^^^
    memset( factories, 0, sizeof(*factories) );
    factories  [0] = SMBIOS::BIOSInformation::Factory;
    factories  [1] = SMBIOS::System::Factory;
    factories  [2] = SMBIOS::BaseBoard::Factory;
    factories  [3] = SMBIOS::Chassis::Factory;
    factories  [4] = SMBIOS::Processor::Factory;
    factories  [7] = SMBIOS::Cache::Factory;
    factories  [8] = SMBIOS::PortConnector::Factory;
    factories  [9] = SMBIOS::SystemSlot::Factory;
    factories [10] = SMBIOS::OnboardDevices::Factory;
    factories [11] = SMBIOS::OEMStrings::Factory;
    factories [16] = SMBIOS::PhysicalMemoryArray::Factory;
    factories [17] = SMBIOS::MemoryDevice::Factory;
    factories [32] = SMBIOS::SystemBoot::Factory;
    factories [41] = SMBIOS::OnboardDevicesExtended::Factory;
    factories[126] = SMBIOS::Inactive::Factory;

    int memfd = open("/dev/mem", O_RDONLY);
    if ( memfd == -1 ) {
        perror( "cannot open mem" );
        exit( 1 );
    }
    void *address = mmap( 0, 0x10000, PROT_READ, MAP_SHARED, memfd, 0xF0000 );
    if ( address == MAP_FAILED ) {
        perror( "cannot map address" );
        exit( 1 );
    }
    if ( debug > 3 ) printf( "mapped SMBIOS\n" );
    close( memfd );

    /*
     * search through this chunk of memory for the SMBIOS header.
     */
    uint8_t *limit = (uint8_t *)address + 0xFFF0;
    uint8_t *p;
    for ( p = (uint8_t *)address ; p < limit ; p += 16 ) {
        if ( p[0] != '_' ) {
            continue;
        }
        if ( p[1] != 'S' ) {
            continue;
        }
        if ( p[2] != 'M' ) {
            continue;
        }
        if ( p[3] != '_' ) {
            continue;
        }
        if ( debug > 3 ) printf( "Found it at %p\n", p );
        goto found_smbios;
    }
    fprintf( stderr, "No SMBIOS table found\n" );
    exit( 1 );

found_smbios:
    return p;
}

/**
 */
void
SMBIOS::Header::print_xml( FILE *f ) {
    if ( table == NULL ) return;

    print_rdf_header( f );

    fprintf( f, "<rdf:Description rdf:about='http://redgates.com/smbios/%s'>", system().uuid()->to_s() );
    fprintf( f, "<rdfs:label>SMBIOS</rdfs:label>" );
    fprintf( f, "<dc:version>%d.%d</dc:version>", major_version, minor_version );
    fprintf( f, "</rdf:Description>" );

    table->print_xml( f );

    print_rdf_trailer( f );
}

/**
 * This is a sample command for testing the straight line netlink
 * probe code.
 */
static int 
Probe_cmd( ClientData data, Tcl_Interp *interp,
             int objc, Tcl_Obj * CONST *objv )
{
    SMBIOS::Header smbios;
    smbios.probe( SMBIOS::Header::locate() );
    return TCL_OK;
}

/**
 * This is a sample command for testing the straight line netlink
 * probe code.
 */
static int 
xml_cmd( ClientData data, Tcl_Interp *interp,
             int objc, Tcl_Obj * CONST *objv )
{
    FILE *output = stdout;

    if ( objc > 2 ) {
        Tcl_ResetResult( interp );
        Tcl_WrongNumArgs( interp, 1, objv, "[filename]" );
    }

    if ( objc == 2 ) {
        char *filename = Tcl_GetStringFromObj( objv[1], NULL );
        output = fopen( filename, "w" );
        if ( output == NULL ) {
            char *mesg = const_cast<char*>("failed to open file");
            Tcl_SetResult( interp, mesg, TCL_STATIC );
            return TCL_ERROR;
        }
    }

    SMBIOS::Header smbios;
    smbios.probe( SMBIOS::Header::locate() );
    smbios.print_xml( output );

    if ( output != stdout ) fclose(output);

    return TCL_OK;
}

/**
 * we can find the SMBIOS address directly by looking through the BIOS data structures directly
 */
bool BIOS::Initialize( Tcl_Interp *interp ) {
    Tcl_Command command;

    using namespace SMBIOS;

    Tcl_Namespace *ns = Tcl_CreateNamespace(interp, "SMBIOS", (ClientData)0, NULL);
    if ( ns == NULL ) {
        return false;
    }

    if ( Tcl_LinkVar(interp, "SMBIOS::debug", (char *)&debug, TCL_LINK_INT) != TCL_OK ) {
        syslog( LOG_ERR, "failed to link SMBIOS::debug" );
        exit( 1 );
    }

    // create TCL commands for creating BIOS/SMBIOS structures
    command = Tcl_CreateObjCommand(interp, "SMBIOS::Probe", Probe_cmd, (ClientData)0, NULL);
    if ( command == NULL ) {
        // syslog ?? want to report TCL Error
        return false;
    }

    command = Tcl_CreateObjCommand(interp, "SMBIOS::xml", xml_cmd, (ClientData)0, NULL);
    if ( command == NULL ) {
        // syslog ?? want to report TCL Error
        return false;
    }

    return true;
}

/*
 * vim:autoindent
 * vim:expandtab
 */
