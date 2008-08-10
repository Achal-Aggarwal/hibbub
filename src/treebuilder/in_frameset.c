/*
 * This file is part of Hubbub.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2008 Andrew Sidwell <takkaria@netsurf-browser.org>
 */

#include <assert.h>
#include <string.h>

#include "treebuilder/modes.h"
#include "treebuilder/internal.h"
#include "treebuilder/treebuilder.h"
#include "utils/utils.h"


/**
 * Handle token in "in frameset" insertion mode
 *
 * \param treebuilder  The treebuilder instance
 * \param token        The token to handle
 * \return True to reprocess token, false otherwise
 */
hubbub_error handle_in_frameset(hubbub_treebuilder *treebuilder,
		const hubbub_token *token)
{
	hubbub_error err = HUBBUB_OK;

	switch (token->type) {
	case HUBBUB_TOKEN_CHARACTER:
		if (process_characters_expect_whitespace(treebuilder,
				token, true)) {
			/** \todo parser error */
		}
		break;
	case HUBBUB_TOKEN_COMMENT:
		process_comment_append(treebuilder, token,
				treebuilder->context.element_stack[
				treebuilder->context.current_node].node);
		break;
	case HUBBUB_TOKEN_DOCTYPE:
		/** \todo parse error */
		break;
	case HUBBUB_TOKEN_START_TAG:
	{
		element_type type = element_type_from_name(treebuilder,
				&token->data.tag.name);

		if (type == HTML) {
			handle_in_body(treebuilder, token);
		} else if (type == FRAMESET) {
			insert_element(treebuilder, &token->data.tag);
		} else if (type == FRAME) {
			insert_element_no_push(treebuilder, &token->data.tag);
			/** \todo ack sc flag */
		} else if (type == NOFRAMES) {
			err = handle_in_head(treebuilder, token);
		} else {
			/** \todo parse error */
		}
	}
		break;
	case HUBBUB_TOKEN_END_TAG:
	{
		element_type type = element_type_from_name(treebuilder,
				&token->data.tag.name);

		if (type == FRAMESET) {
			hubbub_ns ns;
			void *node;

			/* fragment case -- ignore */
			if (current_node(treebuilder) == HTML) {
				/** \todo parse error */
				break;
			}

			if (!element_stack_pop(treebuilder, &ns, &type,
					&node)) {
				/** \todo errors */
			}

			treebuilder->tree_handler->unref_node(
					treebuilder->tree_handler->ctx,
					node);

			if (current_node(treebuilder) != FRAMESET) {
				treebuilder->context.mode = AFTER_FRAMESET;
			}
		} else {
			/** \todo parse error */
		}
	}
		break;
	case HUBBUB_TOKEN_EOF:
		/** \todo parse error */
		break;
	}

	return err;
}

