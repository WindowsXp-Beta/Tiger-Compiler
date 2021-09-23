#include "tiger/frame/temp.h"

#include <cstdio>
#include <set>
#include <sstream>

namespace temp {

LabelFactory LabelFactory::label_factory;
TempFactory TempFactory::temp_factory;

bool TempList::Contain(Temp *t) const {
  return std::any_of(temp_list_.cbegin(), temp_list_.cend(),
                     [t](Temp *temp) { return t->Int() == temp->Int(); });
}

bool TempList::Equal(TempList *tl) const {
  if (!tl || tl->temp_list_.empty())
    return temp_list_.empty();
  auto tmp_left_list = temp_list_;
  auto tmp_right_list = tl->temp_list_;

  tmp_left_list.unique();
  tmp_right_list.unique();

  if (tmp_left_list.size() != tmp_right_list.size())
    return false;

  return std::all_of(tmp_left_list.cbegin(), tmp_left_list.cend(),
                     [tl](Temp *temp) { return tl->Contain(temp); });
}

void TempList::Replace(Temp *old_temp, Temp *new_temp) {
  assert(old_temp && new_temp);
  assert(Contain(old_temp));
  auto it = temp_list_.begin();
  for (; it != temp_list_.end(); it++) {
    if (*it == old_temp) {
      break;
    }
  }
  it = temp_list_.erase(it);
  temp_list_.insert(it, new_temp);
}

void TempList::Delete(Temp *t) { temp_list_.remove(t); }

temp::TempList *TempList::Union(temp::TempList *tl) const {
  temp::TempList tmp = temp::TempList();
  tmp.temp_list_ = std::list<temp::Temp *>(temp_list_);
  if (tl && !tl->temp_list_.empty()) {
    tmp.temp_list_.insert(tmp.temp_list_.end(), tl->temp_list_.begin(),
                          tl->temp_list_.end());
  }
  auto *ret = new temp::TempList();
  std::set<int> temp_set;
  for (auto tmp_temp : tmp.temp_list_) {
    if (temp_set.find(tmp_temp->Int()) != temp_set.end()) {
      continue;
    } else {
      temp_set.insert(tmp_temp->Int());
      ret->temp_list_.push_back(tmp_temp);
    }
  }
  return ret;
}

void TempList::CatList(TempList *tl) {
  if (tl && !tl->temp_list_.empty()) {
    temp_list_.insert(temp_list_.end(), tl->temp_list_.begin(),
                      tl->temp_list_.end());
  }
}

temp::TempList *TempList::Diff(temp::TempList *tl) const {
  auto *res = new temp::TempList();
  if (temp_list_.empty()) {
    return res;
  } else if (!tl || tl->temp_list_.empty()) {
    res->temp_list_ = std::list<Temp *>(temp_list_);
  } else {
    for (auto temp : temp_list_) {
      if (!tl->Contain(temp)) {
        res->temp_list_.push_back(temp);
      }
    }
  }

  return res;
}

Label *LabelFactory::NewLabel() {
  char buf[100];
  sprintf(buf, "L%d", label_factory.label_id_++);
  return NamedLabel(std::string(buf));
}

/**
 * Get symbol of a label_. The label_ will be created only if it is not found.
 * @param s label_ string
 * @return symbol
 */
Label *LabelFactory::NamedLabel(std::string_view s) {
  return sym::Symbol::UniqueSymbol(s);
}

std::string LabelFactory::LabelString(Label *s) { return s->Name(); }

Temp *TempFactory::NewTemp() {
  Temp *p = new Temp(temp_factory.temp_id_++);
  std::stringstream stream;
  stream << p->num_;
  Map::Name()->Enter(p, new std::string(stream.str()));

  return p;
}

int Temp::Int() const { return num_; }

Map *Map::Empty() { return new Map(); }

Map *Map::Name() {
  static Map *m = nullptr;
  if (!m)
    m = Empty();
  return m;
}

Map *Map::LayerMap(Map *over, Map *under) {
  if (over == nullptr)
    return under;
  else
    return new Map(over->tab_, LayerMap(over->under_, under));
}

void Map::Enter(Temp *t, std::string *s) {
  assert(tab_);
  tab_->Enter(t, s);
}

std::string *Map::Look(Temp *t) {
  std::string *s;
  assert(tab_);
  s = tab_->Look(t);
  if (s)
    return s;
  else if (under_)
    return under_->Look(t);
  else
    return nullptr;
}

void Map::DumpMap(FILE *out) {
  tab_->Dump([out](temp::Temp *t, std::string *r) {
    fprintf(out, "t%d -> %s\n", t->Int(), r->data());
  });
  if (under_) {
    fprintf(out, "---------\n");
    under_->DumpMap(out);
  }
}

} // namespace temp