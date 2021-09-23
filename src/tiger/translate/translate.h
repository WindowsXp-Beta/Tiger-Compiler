#ifndef TIGER_TRANSLATE_TRANSLATE_H_
#define TIGER_TRANSLATE_TRANSLATE_H_

#include <list>
#include <memory>

#include "tiger/absyn/absyn.h"
#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/frame.h"
#include "tiger/semant/types.h"

namespace tr {

class Exp;
class ExpAndTy;
class Level;

class Access {
public:
  Level *level_;
  frame::Access *access_;

  Access(Level *level, frame::Access *access)
      : level_(level), access_(access) {}
  static Access *AllocLocal(Level *level, bool escape,
                            frame::RegManager *reg_manager);
};

class Level {
public:
  frame::Frame *frame_;
  Level *parent_;

  Level(frame::Frame *frame, Level *parent) : frame_(frame), parent_(parent) {}
  std::list<tr::Access *> *Formals() {
    /* TODO: Put your lab5 code here */
  }

  static Level *NewLevel(Level *parent, temp::Label *name,
                         std::list<bool> formals,
                         frame::RegManager *reg_manager) {
    /* TODO: Put your lab5 code here */
  }
};

class ProgTr {
public:
  ProgTr(std::unique_ptr<absyn::AbsynTree> absyn_tree,
         std::unique_ptr<err::ErrorMsg> errormsg,
         std::unique_ptr<frame::RegManager> reg_manager)
      : absyn_tree_(std::move(absyn_tree)), errormsg_(std::move(errormsg)),
        reg_manager_(std::move(reg_manager)),
        frags_(std::make_unique<frame::Frags>()),
        main_level_(std::make_unique<Level>(
            frame::NewFrame(temp::LabelFactory::NamedLabel("tigermain"),
                            std::list<bool>(), reg_manager.get()),
            nullptr)),
        tenv_(std::make_unique<env::TEnv>()),
        venv_(std::make_unique<env::VEnv>()) {}

  /**
   * Translate IR tree
   */
  void Translate();

  /**
   * Semantic analysis
   */
  void SemAnalyze();

  /**
   * Transfer the ownership of errormsg to outer scope
   * @return unique pointer to errormsg
   */
  std::unique_ptr<err::ErrorMsg> TransferErrormsg() {
    return std::move(errormsg_);
  }

  /**
   * Transfer the ownership of frags_ to outer scope
   * @return list of frags_
   */
  std::list<std::unique_ptr<frame::Frag>> TransferFrags() {
    return std::move(frags_->TransferFrags());
  }

  /**
   * Transfer the ownership of reg_manager_ to outer scope
   * @return unique pointer to reg_manager_
   */
  std::unique_ptr<frame::RegManager> TransferRegManager() {
    return std::move(reg_manager_);
  }

private:
  std::unique_ptr<absyn::AbsynTree> absyn_tree_;
  std::unique_ptr<err::ErrorMsg> errormsg_;
  std::unique_ptr<frame::RegManager> reg_manager_;

  std::unique_ptr<frame::Frags> frags_;
  std::unique_ptr<Level> main_level_;
  std::unique_ptr<env::TEnv> tenv_;
  std::unique_ptr<env::VEnv> venv_;

  // Fill base symbol for var env and type env
  void FillBaseVEnv();
  void FillBaseTEnv();
};

} // namespace tr

#endif
