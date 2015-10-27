#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/ec.h>

#include "block.h"
#include "common.h"
#include "transaction.h"

/* Usage: ./balances *.blk
 * Reads in a list of block files and outputs a table of public key hashes and
 * their balance in the longest chain of blocks. In case there is more than one
 * chain of the longest length, chooses one arbitrarily. */

/* If a block has height 0, it must have this specific hash. */
const hash_output GENESIS_BLOCK_HASH = {
	0x00, 0x00, 0x00, 0x0e, 0x5a, 0xc9, 0x8c, 0x78, 0x98, 0x00, 0x70, 0x2a, 0xd2, 0xa6, 0xf3, 0xca,
	0x51, 0x0d, 0x40, 0x9d, 0x6c, 0xca, 0x89, 0x2e, 0xd1, 0xc7, 0x51, 0x98, 0xe0, 0x4b, 0xde, 0xec,
};

struct blockchain_node {
	struct blockchain_node *parent;
	struct block b;
	int is_valid;
};


/* A simple linked list to keep track of account balances. */
struct balance {
	struct ecdsa_pubkey pubkey;
	int balance;
	struct balance *next;
};

/* Add or subtract an amount from a linked list of balances. Call it like this:
 *   struct balance *balances = NULL;
 *
 *   // reward_tx increment.
 *   balances = balance_add(balances, &b.reward_tx.dest_pubkey, 1);
 *
 *   // normal_tx increment and decrement.
 *   balances = balance_add(balances, &b.normal_tx.dest_pubkey, 1);
 *   balances = balance_add(balances, &prev_transaction.dest_pubkey, -1);
 */
static struct balance *balance_add(struct balance *balances,
	struct ecdsa_pubkey *pubkey, int amount)
{
	struct balance *p;

	for (p = balances; p != NULL; p = p->next) {
		if ((byte32_cmp(p->pubkey.x, pubkey->x) == 0)
			&& (byte32_cmp(p->pubkey.y, pubkey->y) == 0)) {
			p->balance += amount;
			return balances;
		}
	}

	/* Not found; create a new list element. */
	p = malloc(sizeof(struct balance));
	if (p == NULL)
		return NULL;
	p->pubkey = *pubkey;
	p->balance = amount;
	p->next = balances;

	return p;
}

//Returns a pointer to a blockchain node instance
struct blockchain_node* make_node(struct block b, struct blockchain_node* parent)
{
	struct blockchain_node *node; 
	node = malloc(sizeof(struct blockchain_node));
	if (node == NULL)
		return NULL;
	node->b = b;
	node->parent = parent;
	node->is_valid = -1;
	return node;
}


int cmp_block(const void* ia, const void* ib) {
	struct blockchain_node* a = (struct blockchain_node*) ia;
	struct blockchain_node* b = (struct blockchain_node*) ib;
	if(a->b.height < b->b.height) {
		return -1;
	} else if (a->b.height > b->b.height) {
		return 1;
	} else {
		return 0;
	}
}

//Orders the tree from the given root
//struct blockchain_node* order_tree(struct blockchain_node* root) {

//}


int main(int argc, char *argv[])
{
	int i;

	//This will have all of our pinters to blocks
	struct blockchain_node *block_arr[argc];
	//block_arr = malloc(sizeof(struct blockchain_node) * argc);

	//This will point to the root of our tree
	//	struct blockchain_node* root;
	//struct blockchain_node* last;

	/* Read input block files. */
	for (i = 1; i < argc; i++) {
		char *filename;
		struct block b;
		int rc;

		filename = argv[i];
		rc = block_read_filename(&b, filename);
		if (rc != 1) {
			fprintf(stderr, "could not read %s\n", filename);
			exit(1);
		}
		printf("Read block %d\n", b.height);
		/* TODO READ BLOCKS INTO MEMORY*/
		//Just read everything in first
		block_arr[i] = make_node(b, 0);


		/* Feel free to add/modify/delete any code you need to. */
	}

	/* Organize into a tree, check validity, and output balances. */
	/* TODO */
	qsort(block_arr, sizeof(block_arr) / sizeof(struct blockchain_node), sizeof(struct blockchain_node), cmp_block);


	struct balance *balances = NULL, *p, *next;
	/* Print out the list of balances. */
	for (p = balances; p != NULL; p = next) {
		next = p->next;
		printf("%s %d\n", byte32_to_hex(p->pubkey.x), p->balance);
		free(p);
	}

	return 0;
}
