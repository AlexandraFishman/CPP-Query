#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
using namespace std;
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<QueryBase> QueryBase::factory(const string& s)
{
  regex words_regex("[\\w']+");
  auto matchesRegex = sregex_iterator(s.begin(), s.end(), words_regex);
  if(s == matchesRegex->str()) 
        return std::shared_ptr<QueryBase>(new WordQuery(matchesRegex->str()));
  
  regex not_words_regex("NOT [\\w']+");
  matchesRegex = sregex_iterator(s.begin(), s.end(), not_words_regex);
  if(s == matchesRegex->str()){
    // cout << matchesRegex->str();
        return std::shared_ptr<QueryBase>(new NotQuery(matchesRegex->str().substr(4,matchesRegex->str().length())));
  } 
  // if(s.equals(wo))
}
////////////////////////////////////////////////////////////////////////////////
QueryResult NotQuery::eval(const TextQuery &text) const
{
  QueryResult result = text.query(query_word);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto beg = result.begin(), end = result.end();
  auto sz = result.get_file()->size();
  
  for (size_t n = 0; n != sz; ++n)
  {
    if (beg==end || *beg != n)
		ret_lines->insert(n);
    else if (beg != end)
		++beg;
  }
  return QueryResult(rep(), ret_lines, result.get_file());
    
}

QueryResult AndQuery::eval (const TextQuery& text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set_intersection(left_result.begin(), left_result.end(),
      right_result.begin(), right_result.end(), 
      std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = 
      std::make_shared<std::set<line_no>>(left_result.begin(), left_result.end());

  ret_lines->insert(right_result.begin(), right_result.end());

  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////
QueryResult NQuery::eval(const TextQuery &text) const
{
      // regex txt_regex("");
      // regex_match(text, txt_regex);
}
/////////////////////////////////////////////////////////