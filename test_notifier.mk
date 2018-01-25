OBJS.test_notifier = test_notifier.o
EXEC.test_notifier = test_notifier$X
$(EXEC.test_notifier) : $(OBJS.test_notifier)
clean :: ; $(RM) $(EXEC.test_notifier) $(OBJS.test_notifier)
all :: $(EXEC.test_notifier)
