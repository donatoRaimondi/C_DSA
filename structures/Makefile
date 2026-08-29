STRUCTURES := list ordered_list stack queue set dictionary b_tree priority_queue bst

.PHONY: all test sanitize clean $(STRUCTURES)

all: $(STRUCTURES)

$(STRUCTURES):
	$(MAKE) -C $@

test:
	@for structure in $(STRUCTURES); do \
		$(MAKE) -C $$structure test || exit $$?; \
	done

sanitize:
	@for structure in $(STRUCTURES); do \
		$(MAKE) -C $$structure sanitize || exit $$?; \
	done

clean:
	@for structure in $(STRUCTURES); do \
		$(MAKE) -C $$structure clean || exit $$?; \
	done
