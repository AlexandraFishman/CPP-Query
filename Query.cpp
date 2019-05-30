/* Alex Fishman 319451514 */
/* Elad Motzny 204093694 */

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
  //All of the required regex:
  regex words_regex("[\\w']+");
  regex not_words_regex("NOT [\\w']+");
  regex and_words_regex("[\\w']+ AND [\\w']+");
  regex or_words_regex("[\\w']+ OR [\\w']+");
  regex n_words_regex("[\\w']+ [\\d]+ [\\w']+");
  //word
  if(regex_match(s,words_regex)){
    //cout<<"word"<<endl;
    auto matchesRegex = sregex_iterator(s.begin(), s.end(), words_regex);
    return std::shared_ptr<QueryBase>(new WordQuery(matchesRegex->str()));
  }
  //not word
  else if(regex_match(s,not_words_regex)){
    //cout<<"not"<<endl;
    auto matchesRegex = sregex_iterator(s.begin(), s.end(), not_words_regex);
    return std::shared_ptr<QueryBase>(new NotQuery(matchesRegex->str().substr(4,matchesRegex->str().length())));
  }
  //word and word
  else if(regex_match(s,and_words_regex)){
    //cout<<"and"<<endl;
    auto matchesRegex = sregex_iterator(s.begin(), s.end(), and_words_regex);
    auto l = sregex_iterator(s.begin(), s.end(), words_regex);
    auto r = sregex_iterator(s.begin(), s.end(), words_regex);
    r++;// AND
    r++;// second word
    return std::shared_ptr<QueryBase>(new AndQuery(l->str(), r->str()));
  }
  //word or word
  else if(regex_match(s,or_words_regex)){
    //cout<<"or"<<endl;
    auto matchesRegex = sregex_iterator(s.begin(), s.end(), or_words_regex);
    auto l = sregex_iterator(s.begin(), s.end(), words_regex);
    auto r = sregex_iterator(s.begin(), s.end(), words_regex);
    r++;// OR
    r++;// second word
    return std::shared_ptr<QueryBase>(new OrQuery(l->str(), r->str()));
  }
  //word n word
  else if(regex_match(s,n_words_regex)){
    //cout<<"n"<<endl;
    auto matchesRegex = sregex_iterator(s.begin(), s.end(), n_words_regex);
    auto l = sregex_iterator(s.begin(), s.end(), words_regex);
    auto r = sregex_iterator(s.begin(), s.end(), words_regex);
    r++;// n
    r++;// second word
    auto n = sregex_iterator(s.begin(), s.end(), words_regex);
    n++;
    return std::shared_ptr<QueryBase>(new NQuery(l->str(), r->str(), stoi(n->str())));
  }
  //Unrecognized search
  else{
    throw invalid_argument ("Unrecognized search");//didnt get into any one of the ifs, throw an invalid argument
  }
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
QueryResult NQuery::eval(const TextQuery &text) const{
  QueryResult QR = AndQuery::eval(text);
  regex firstWord(left_query);
  regex secondWord(right_query);
  auto ret_lines = std::make_shared<std::set<line_no>>();//where we return our lines
  auto iter = QR.begin();
  //Need regex to seperate the spaces between words and then I can count the number of words in between the 2 given words 
  //lets go over the lines:
  for (; iter != QR.end(); ++iter){
    string currLine = QR.get_file()->at(*iter);
    //cout << "Current line: " << currLine << endl;
    std::smatch match;
    regex_search(currLine, match, firstWord);
    int firstPos=match.position(0);//get position of the first word
    //cout << "first word position: " << firstPos <<endl;
    int i=firstPos + match.length();
    regex_search(currLine,match,secondWord);
    int secondPos=match.position(0);//get position of the second word
    //cout << "second word position: " << secondPos <<endl;
    int numOfWords=0;
    bool isBlank = true;
    while(i < secondPos-1){
      //cout << "current word: "<< currLine[i] <<endl;
      if(std::isspace(currLine[i])){
        isBlank = true;
      }
      else if(isBlank){
        numOfWords++;
        isBlank = false;
        //cout << "num of current words: " << numOfWords << endl;
      }
      ++i;
    }
    if(numOfWords <= dist){
      ret_lines->insert(*iter);
    }
  }
  return QueryResult(rep(), ret_lines, QR.get_file());
}
/////////////////////////////////////////////////////////