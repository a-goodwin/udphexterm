#ifndef CONFIGUTILS_H
#define CONFIGUTILS_H

#define InitConf(st, mod) st = new QSettings(this); \
                                   st->beginGroup(mod)

#define Conf2UiStr(st, uiel) if (st->contains(uiel->objectName())) \
    uiel->setText(st->value(uiel->objectName()).toString())
#define C2US(st, uiel) Conf2UiStr(st, uiel)

#define Conf2UiChecked(st, uiel) if (st->contains(uiel->objectName())) \
    uiel->setChecked(st->value(uiel->objectName()).toBool())
#define C2UC(st, uiel) Conf2UiChecked(st, uiel)

#define Conf2UiInt(st, uiel) if (st->contains(uiel->objectName())) \
    uiel->setValue(st->value(uiel->objectName()).toInt())
#define C2UI(st, uiel) Conf2UiInt(st, uiel)

#define Conf2UiDbl(st, uiel) if (st->contains(uiel->objectName())) \
    uiel->setValue(st->value(uiel->objectName()).toDouble())
#define C2UD(st, uiel) Conf2UiDbl(st, uiel)


#define Ui2ConfStr(st, uiel) st->setValue(uiel->objectName(), uiel->text())
#define U2CS(st, uiel) Ui2ConfStr(st, uiel)
#define Ui2ConfChecked(st, uiel) st->setValue(uiel->objectName(), uiel->isChecked())
#define U2CC(st, uiel) Ui2ConfChecked(st, uiel)
#define Ui2ConfInt(st, uiel) st->setValue(uiel->objectName(), uiel->value())
#define U2CI(st, uiel) Ui2ConfInt(st, uiel)
#define Ui2ConfDbl(st, uiel) st->setValue(uiel->objectName(), uiel->value())
#define U2CD(st, uiel) Ui2ConfDbl(st, uiel)

#define ConfCommit(st) st->sync()

#endif // CONFIGUTILS_H
