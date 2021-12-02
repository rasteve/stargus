/*
 * DataHub.cpp
 *
 *      Author: Andreas Volz
 */

// Local
#include <StatTxtTbl.h>
#include "DataHub.h"
#include "Hurricane.h"
#include "StringUtil.h"

// System
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

DataHub::DataHub(std::shared_ptr<Hurricane> hurricane) :
	Converter(hurricane),
	mLogger("startool.DataHub")
{

}

DataHub::~DataHub()
{

}

std::shared_ptr<kaitai::kstream> DataHub::getKaitaiStream(const std::string &file)
{
	shared_ptr<DataChunk> data = mHurricane->extractDataChunk(file);
	if (data)
	{
		//std::ifstream ifs(arcfile, std::ifstream::binary);
		//kaitai::kstream ks(&ifs);

		// TODO: for now just create from complete string instead of istream. Maybe change the complete
		// reader to stream based concept...
		std::string str( reinterpret_cast<char const*>(data->getDataPointer()), data->getSize() ) ;
		shared_ptr<kaitai::kstream> ks = make_shared<kaitai::kstream>(str);
		return ks;
	}

	// TODO: hm, better create an exception
	return nullptr;
}

bool DataHub::convert(const std::string &arcfile, const std::string &file)
{
	printCSV();

	return true;
}

void DataHub::printCSV()
{
	string csv_dat;
	char buf[1024];
	const string CSV_ENDLINE = "\n";
	const string CSV_SEPARATOR = ";";

	string sc_rez_stat_txt_tbl ="rez\\stat_txt.tbl";
	string sc_arr_units_dat = "arr\\units.dat";
	string sc_arr_weapons_dat = "arr\\weapons.dat";
	string sc_arr_flingy_dat = "arr\\flingy.dat";
	string sc_arr_sprites_dat = "arr\\sprites.dat";
	string sc_arr_images_dat = "arr\\images.dat";
	string sc_arr_images_tbl = "arr\\images.tbl";


	// stat_txt.tbl
	StatTxtTbl stat_txt;
	shared_ptr<kaitai::kstream> stat_txt_ks = getKaitaiStream(sc_rez_stat_txt_tbl);
	std::vector<TblEntry> stat_txt_vec = stat_txt.convertFromStream(stat_txt_ks);

	// units.dat
	shared_ptr<kaitai::kstream> units_ks = getKaitaiStream(sc_arr_units_dat);
	units_dat_t units = units_dat_t(false, false, false, units_ks.get());
	std::vector<uint8_t>* units_graphics_vec = units.graphics();
	std::vector<uint8_t>* units_ground_weapon_vec = units.ground_weapon();
	std::vector<uint8_t>* units_air_weapon_vec = units.air_weapon();
	std::vector<units_dat_t::staredit_group_flags_type_t*>* se_group_flags_vec = units.staredit_group_flags();

	// weapons.dat
	int ground_weapon_max = *max_element(units_ground_weapon_vec->begin(), units_ground_weapon_vec->end());
	int air_weapon_max = *max_element(units_air_weapon_vec->begin(), units_air_weapon_vec->end());
	int weapon_max = ground_weapon_max;
	if(air_weapon_max > ground_weapon_max)
		weapon_max = air_weapon_max;
	shared_ptr<kaitai::kstream> weapons_ks = getKaitaiStream(sc_arr_weapons_dat);
	//configure weapons parser with max size of weapons
	printf("weapon_max=%d\n", weapon_max);
	weapons_dat_t weapons = weapons_dat_t(weapon_max, weapons_ks.get());
	std::vector<uint16_t>* weapon_label_vec = weapons.label();
	std::vector<uint32_t>* weapon_graphics_vec = weapons.graphics();
	std::vector<uint16_t>* weapon_error_message_vec = weapons.error_message();

	// flingy.dat
	shared_ptr<kaitai::kstream> flingy_ks = getKaitaiStream(sc_arr_flingy_dat);
	int unit_graphics_max = *max_element(units_graphics_vec->begin(), units_graphics_vec->end());
	printf("unit_graphics_max=%d\n", unit_graphics_max);
	int weapon_graphics_max = *max_element(weapon_graphics_vec->begin(), weapon_graphics_vec->end());
	printf("weapon_graphics_max=%d\n", weapon_graphics_max);
	int graphics_max = unit_graphics_max;
	if(weapon_graphics_max > unit_graphics_max)
		graphics_max = weapon_graphics_max;
	// add +1 because start counting from 0
	flingy_dat_t flingy = flingy_dat_t(graphics_max+1, flingy_ks.get());
	std::vector<uint16_t>* flingy_sprites_vec = flingy.sprite();

	// sprites.dat
	shared_ptr<kaitai::kstream> sprites_ks = getKaitaiStream(sc_arr_sprites_dat);
	int flingy_sprites_max = *max_element(flingy_sprites_vec->begin(), flingy_sprites_vec->end());
	printf("flingy_sprites_max=%d\n", flingy_sprites_max);
	// add +1 because start counting from 0
	sprites_dat_t sprites = sprites_dat_t(flingy_sprites_max+1, sprites_ks.get());
	std::vector<uint16_t>* sprites_images_vec = sprites.image_file();

	// images.dat
	shared_ptr<kaitai::kstream> images_ks = getKaitaiStream(sc_arr_images_dat);
	int sprites_image_file_max = *max_element(sprites_images_vec->begin(), sprites_images_vec->end());
	printf("sprites_image_file_max=%d\n", sprites_image_file_max);
	// add +1 because start counting from 0
	images_dat_t images = images_dat_t(sprites_image_file_max+1, images_ks.get());
	std::vector<uint32_t>* images_grp_vec = images.grp_file();

	// images.tbl
	StatTxtTbl images_tbl;
	shared_ptr<kaitai::kstream> images_tbl_ks = getKaitaiStream(sc_arr_images_tbl);
	std::vector<TblEntry> images_tbl_vec = stat_txt.convertFromStream(images_tbl_ks);

	// Units.dat
	for(unsigned int i = 0; i < units_graphics_vec->size(); i++)
	{
		csv_dat += "units.dat";

		csv_dat += CSV_SEPARATOR;

		csv_dat += "id=" + toString(i);

		csv_dat += CSV_SEPARATOR;

		uint8_t graphic_id = units_graphics_vec->at(i);
		sprintf(buf, "graphics=%d", graphic_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		units_dat_t::staredit_group_flags_type_t *se_group_flags = se_group_flags_vec->at(i);
		bool zerg = se_group_flags->zerg();
		bool terran = se_group_flags->terran();
		bool protoss = se_group_flags->protoss();

		if(zerg)
		{
			csv_dat += "race=zerg";
		}
		else if(terran)
		{
			csv_dat += "race=terran";
		}
		else if(protoss)
		{
			csv_dat += "race=protoss";
		}

		csv_dat += CSV_SEPARATOR;

		TblEntry tblEntry = stat_txt_vec.at(i);
		csv_dat += "ref:name=" + tblEntry.name;

		csv_dat += CSV_SEPARATOR;

		uint16_t sprite_id = flingy_sprites_vec->at(graphic_id);
		sprintf(buf, "ref:sprite=%d", sprite_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		uint16_t weapon_id = units_ground_weapon_vec->at(i);
		sprintf(buf, "weapon=%d", weapon_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		csv_dat += CSV_ENDLINE;
	}

	// weapons.dat
	for(unsigned int i = 0; i < weapon_label_vec->size(); i++)
	{
		csv_dat += "weapons.dat";

		csv_dat += CSV_SEPARATOR;

		csv_dat += "id=" + toString(i);

		csv_dat += CSV_SEPARATOR;

		uint16_t label_id = weapon_label_vec->at(i);
		sprintf(buf, "label=%d", label_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		TblEntry tblEntry_label = stat_txt_vec.at(label_id);
		csv_dat += "ref:name=" + tblEntry_label.name;

		csv_dat += CSV_SEPARATOR;

		uint32_t graphic = weapon_graphics_vec->at(i);
		sprintf(buf, "graphics=%d", graphic);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		uint16_t error_id = weapon_error_message_vec->at(i);
		sprintf(buf, "error_id=%d", error_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		TblEntry tblEntry_error = stat_txt_vec.at(error_id);
		csv_dat +=" ref:error_text=" + tblEntry_error.name;

		csv_dat += CSV_SEPARATOR;

		csv_dat += CSV_ENDLINE;
	}

	// flingy.dat
	for(unsigned int i = 0; i < flingy_sprites_vec->size(); i++)
	{
		csv_dat += "flingy.dat";

		csv_dat += CSV_SEPARATOR;

		csv_dat += "id=" + toString(i);

		csv_dat += CSV_SEPARATOR;

		uint16_t flingy_id = flingy_sprites_vec->at(i);
		sprintf(buf, "sprite=%d", flingy_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		csv_dat += CSV_ENDLINE;
	}

	// sprites.dat
	for(unsigned int i = 0; i < sprites_images_vec->size(); i++)
	{
		csv_dat += "sprites.dat";

		csv_dat += CSV_SEPARATOR;

		csv_dat += "id=" + toString(i);

		csv_dat += CSV_SEPARATOR;

		uint16_t image_id = sprites_images_vec->at(i);
		sprintf(buf, "images_file=%d", image_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		csv_dat += CSV_ENDLINE;
	}

	// images.dat
	for(unsigned int i = 0; i < images_grp_vec->size(); i++)
	{
		csv_dat += "images.dat";

		csv_dat += CSV_SEPARATOR;

		csv_dat += "id=" + toString(i);

		csv_dat += CSV_SEPARATOR;

		uint16_t grp_id = images_grp_vec->at(i);
		sprintf(buf, "grp_file=%d", grp_id);
		csv_dat += buf;

		csv_dat += CSV_SEPARATOR;

		TblEntry tblEntry = images_tbl_vec.at(grp_id-1); // spec says first index is -1
		csv_dat += "ref:name=" + tblEntry.name;

		csv_dat += CSV_SEPARATOR;

		csv_dat += CSV_ENDLINE;
	}

	// stdout
	cout << csv_dat;
}
