/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2011, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "history_log.hh"

#include <libxml/xmlreader.h>
#ifndef LIBXML_READER_ENABLED
#error "LIBXML_READER_ENABLED is needed"
#endif

#include "helper.hh"

History_log::History_log(Log& l, std::string params) :
  History(l,params), act(NULL), tag(NULL), c(NULL), a(NULL), file(params)
{

}

void History_log::processNode(xmlTextReaderPtr reader)
{
  char* name =(char*) xmlTextReaderConstName(reader);
  if (name==NULL) return;

  if ((xmlTextReaderDepth(reader)==3) &&
      (strcmp((const char*)name,"action")==0)) {
    // action
    if (act) {
      send_action();
    }
    act=(char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"name");
  }
  
  if ((xmlTextReaderDepth(reader)==3) &&
      (strcmp((const char*)name,"tags")==0)) {
    // tags
    tag=(char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"enabled");
    if (act) {
      send_action();      
    }
   }
  
  if ((xmlTextReaderDepth(reader)==3) &&
      (strcmp((const char*)name,"stop")==0)) {
    if (act) {
      send_action();
    }
    // verdict
    char* ver=(char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"verdict");
  }
}

void History_log::set_coverage(Coverage* cov,
			       Alphabet* alpha)
{
  c=cov;
  a=alpha;

  xmlTextReaderPtr reader =
    xmlReaderForFile(file.c_str(), NULL, 0);
  
  if (reader != NULL) {
    int ret;
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
      processNode(reader);
      ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
  } 
}

void History_log::send_action()
{
  std::string a(act);
  std::string t(tag);

  std::string separator(" ");
  std::vector<std::string> props;
  
  strvec(props,t,separator);
  send_action(a,props);
  free(tag);
  free(act);
  tag=NULL;
  act=NULL;
}

bool History_log::send_action(std::string& act,
			      std::vector<std::string>& props)
{
 std::vector<int> p;

  if (c&&a) {
    if (act=="pass") {
      c->history(0,p,Verdict::PASS);
      return true;
    } 

    if (act=="fail") {
      c->history(0,p,Verdict::FAIL);
      return true;
    }

    if (act=="inconclusive") {
      c->history(0,p,Verdict::INCONCLUSIVE);
      return true;
    }

    if (act=="error") {
      c->history(0,p,Verdict::ERROR);
      return true;
    }

    if (act=="undefined") {
      c->history(0,p,Verdict::UNDEFINED);
      return true;
    }
    
    int action=find(a->getActionNames(),act);

    if (action>0) {

      for(unsigned i=0;i<props.size();i++) {
	int j=find(a->getSPNames(),props[i]);
	p.push_back(j);
      }

      c->history(action,p,Verdict::UNDEFINED);
      return true;
    } else {
      // Tau?
    }
  }

  return false;  
}

FACTORY_DEFAULT_CREATOR(History, History_log, "log")