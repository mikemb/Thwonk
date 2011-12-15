bin_PROGRAMS = mailinject rulerunner msgdelivery
mailinject_SOURCES = mailinject.c codewide.c setupthang.c dbchatter.c parsemail.c mngmail.c msgqueue.c void.c logerror.c user.c misc.c sandbox.c message.c 
rulerunner_SOURCES = rulerunner.c jsrunner.c sandbox.c codewide.c setupthang.c dbchatter.c logerror.c msgqueue.c jsthwonk.c mnglogic.c mngvfile.c misc.c message.c mngmail.c parsemail.c void.c user.c 
msgdelivery_SOURCES = msgdelivery.c sandbox.c codewide.c setupthang.c dbchatter.c logerror.c msgqueue.c misc.c message.c mngmail.c parsemail.c void.c user.c
INCLUDES = -Wall $(MYSQL_CFLAGS) $(SPIDERMONKEY_CFLAGS) $(MAILUTILS_CFLAGS)
LIBS = $(MYSQL_LIBS) $(SPIDERMONKEY_LIBS) $(MAILUTILS_LIBS)
