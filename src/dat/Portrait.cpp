/*
 * Portrait.cpp
 *
 *      Author: Andreas Volz
 */

#include "Portrait.h"
#include "Logger.h"
#include "StringUtil.h"
#include "platform.h"

using namespace std;

namespace dat
{

static Logger logger = Logger("startool.dat.Portrait");

Portrait::Portrait(DataHub &datahub,  uint16_t id) :
  mDatahub(datahub),
  mId(id)
{

}

Portrait::~Portrait()
{

}

uint32_t Portrait::video(bool talking)
{
  uint16_t id = mId;

  if(talking)
    mId += 110;

  uint32_t portrait_file_id = mDatahub.portrait->video()->at(id);
  LOG4CXX_TRACE(logger, string("file(") + to_string(portrait_file_id) + ")");

  return portrait_file_id;
}

TblEntry Portrait::idle()
{
  TblEntry tbl_entry = mDatahub.portdata_tbl_vec.at(video() - 1);
  LOG4CXX_TRACE(logger, string("idle(") + tbl_entry.name1 + ")");

  return tbl_entry;
}

TblEntry Portrait::talking()
{
  TblEntry tbl_entry = mDatahub.portdata_tbl_vec.at(video(true) - 1);
  LOG4CXX_TRACE(logger, string("talking(") + tbl_entry.name1 + ")");

  return tbl_entry;
}

std::string Portrait::PathToID(const std::string &portrait)
{
  string portrait_name(portrait);
  replaceString("\\", "/", portrait_name);
  fs::path p(portrait_name);

  portrait_name = to_lower(p.parent_path().string());
  //cout << "portrait_name: " << portrait_name << endl;

  return portrait_name;
}

} /* namespace dat */
