/*
 * test.c — pwdman test suite
 *
 * Tests all commands (add, update, delete, find, list) and their
 * error paths: invalid emails, invalid sites, invalid IDs, missing
 * records, and SQL injection attempts.
 *
 * Build:
 *   make test
 *
 * Run:
 *   ./test
 *
 * The test binary uses a dedicated test database (.pwdman_test.db)
 * so it never touches your real data. It is wiped and recreated
 * fresh on every run.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>

#include <pwdman.h>
#include <request.h>
#include <resdef.h>
#include <iter.h>
#include <list.h>
#include <validator.h>
#include <database.h>
#include <crypto.h>

/* -------------------------------------------------------------------------
 * Test framework
 * ---------------------------------------------------------------------- */

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)           \
	do                         \
	{                          \
		printf("  %-55s", name); \
		tests_run++;             \
	} while (0)

#define PASS()           \
	do                     \
	{                      \
		printf("✅ PASS\n"); \
		tests_passed++;      \
	} while (0)

#define FAIL(reason)                                      \
	do                                                      \
	{                                                       \
		printf("❌ FAIL — %s (line %d)\n", reason, __LINE__); \
		tests_failed++;                                       \
	} while (0)

#define EXPECT_TRUE(expr, reason) \
	do                              \
	{                               \
		if (!(expr))                  \
		{                             \
			FAIL(reason);               \
		}                             \
		else                          \
		{                             \
			PASS();                     \
		}                             \
	} while (0)

#define EXPECT_FALSE(expr, reason) \
	do                               \
	{                                \
		if ((expr))                    \
		{                              \
			FAIL(reason);                \
		}                              \
		else                           \
		{                              \
			PASS();                      \
		}                              \
	} while (0)

#define EXPECT_EQ(a, b, reason) \
	do                            \
	{                             \
		if ((a) != (b))             \
		{                           \
			FAIL(reason);             \
		}                           \
		else                        \
		{                           \
			PASS();                   \
		}                           \
	} while (0)

#define EXPECT_STR_EQ(a, b, reason) \
	do                                \
	{                                 \
		if (strcmp((a), (b)) != 0)      \
		{                               \
			FAIL(reason);                 \
		}                               \
		else                            \
		{                               \
			PASS();                       \
		}                               \
	} while (0)

#define SECTION(title) \
	printf("\n── %s\n", title)

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

static void destroyer(void *data)
{
	struct pwdman *p = (struct pwdman *)data;
	free(p->site);
	free(p->email);
	free(p->password);
	free(p);
}

/* Build a request with a URI and up to 4 key=value param pairs.
 * Pass NULL to terminate the key list early. */
static struct request *make_req(
	const char *uri,
	const char *k1, const char *v1,
	const char *k2, const char *v2,
	const char *k3, const char *v3,
	const char *k4, const char *v4
)
{
	struct request *req = calloc(1, sizeof(struct request));
	request_uri_set(req, uri);
	if (k1)
		request_param_set(req, k1, v1);
	if (k2)
		request_param_set(req, k2, v2);
	if (k3)
		request_param_set(req, k3, v3);
	if (k4)
		request_param_set(req, k4, v4);
	return req;
}

static void free_req(struct request *req)
{
	free(req);
}

/* Returns the number of entries currently in the info table. */
static int db_count(void)
{
	List *all = pwdman_print_all();
	if (!all) return 0;
	int count = (int)all->size;
	list_destroy(&all);

	return count;
}

/* -------------------------------------------------------------------------
 * Test database setup
 *
 * We override the DB name by deleting and recreating it before the suite.
 * The crypto layer must also be initialised with a fixed test key so
 * encrypt/decrypt round-trips work without a real passphrase prompt.
 * ---------------------------------------------------------------------- */

/* Defined in crypto.c — exposed only for the test binary */
extern bool crypto_init_with_key(const unsigned char *key, int keylen);

static void setup_test_db(void)
{
	/* Remove stale test DB so every run starts clean */
	unlink(".pwdman_test.db");
	database_create_app();
}

/* -------------------------------------------------------------------------
 * Suite: validator
 * ---------------------------------------------------------------------- */

static void suite_validator(void)
{
	SECTION("Validator");

	TEST("valid email accepted");
	EXPECT_TRUE(validate_email("user@example.com"), "should be valid");

	TEST("valid email with subdomain accepted");
	EXPECT_TRUE(validate_email("user@mail.example.com"), "should be valid");

	TEST("email missing @ rejected");
	EXPECT_FALSE(validate_email("userexample.com"), "missing @");

	TEST("email missing domain rejected");
	EXPECT_FALSE(validate_email("user@"), "missing domain");

	TEST("email missing local part rejected");
	EXPECT_FALSE(validate_email("@example.com"), "missing local part");

	TEST("email with spaces rejected");
	EXPECT_FALSE(validate_email("user @example.com"), "space in email");
	
	TEST("empty email rejected");
	EXPECT_FALSE(validate_email(""), "empty string");

	TEST("valid site accepted");
	EXPECT_TRUE(validate_site("github.com"), "should be valid");

	TEST("valid site with subdomain accepted");
	EXPECT_TRUE(validate_site("mail.google.com"), "should be valid");

	TEST("site missing TLD rejected");
	EXPECT_FALSE(validate_site("github"), "no dot");

	TEST("site with spaces rejected");
	EXPECT_FALSE(validate_site("git hub.com"), "space in site");

	TEST("empty site rejected");
	EXPECT_FALSE(validate_site(""), "empty string");

	TEST("valid number accepted");
	EXPECT_TRUE(validate_number("42"), "should be valid");

	TEST("zero accepted as valid number");
	EXPECT_TRUE(validate_number("0"), "zero is valid");

	TEST("negative number rejected");
	EXPECT_FALSE(validate_number("-1"), "negative not valid id");

	TEST("alpha string rejected as number");
	EXPECT_FALSE(validate_number("abc"), "not a number");

	TEST("alphanumeric string rejected as number");
	EXPECT_FALSE(validate_number("12abc"), "mixed string not a number");

	TEST("empty string rejected as number");
	EXPECT_FALSE(validate_number(""), "empty string");
}

/* -------------------------------------------------------------------------
 * Suite: add
 * ---------------------------------------------------------------------- */

static void suite_add(void)
{
	SECTION("Add");

	struct request *req;
	int before, after;

	TEST("add valid entry succeeds");
	req = make_req("/add",
		"site", "github.com",
		"email", "alice@example.com",
		"password", "hunter2",
		NULL, NULL
	);
	EXPECT_TRUE(pwdman_add(req), "add should return true");
	free_req(req);

	TEST("add second valid entry succeeds");
	req = make_req("/add",
		"site", "gitlab.com",
		"email", "alice@example.com",
		"password", "p@ssw0rd!",
		NULL, NULL
	);
	EXPECT_TRUE(pwdman_add(req), "add should return true");
	free_req(req);

	TEST("add third entry with different email");
	req = make_req("/add",
		"site", "bitbucket.org",
		"email", "bob@work.com",
		"password", "correct-horse-battery",
		NULL, NULL
	);
	EXPECT_TRUE(pwdman_add(req), "add should return true");
	free_req(req);

	/* SQL injection attempts via add */
	TEST("SQL injection in site field is stored safely");
	before = db_count();
	req = make_req("/add",
		"site", "evil.com'; DROP TABLE info; --",
		"email", "evil@hack.com",
		"password", "payload",
		NULL, NULL
	);
	bool added = pwdman_add(req);
	after = db_count();
	free_req(req);
	/* Either the validator rejects it (added=false, count unchanged)
	 * OR it's stored safely as literal text (count increased by 1).
	 * What must NOT happen: the table gets dropped (count goes to 0). */
	EXPECT_TRUE(after >= before, "info table must survive injection attempt");

	TEST("SQL injection in password field is stored safely");
	before = db_count();
	req = make_req("/add",
		"site", "safe.com",
		"email", "safe@safe.com",
		"password", "'); DELETE FROM info; --",
		NULL, NULL
	);
	pwdman_add(req);
	after = db_count();
	free_req(req);
	EXPECT_TRUE(after >= before, "info table must survive injection attempt");
}

/* -------------------------------------------------------------------------
 * Suite: find
 * ---------------------------------------------------------------------- */

static void suite_find(void)
{
	SECTION("Find");

	struct request *req;
	List *list;
	struct pwdman *entry;
	Iter *iter;

	/* --- find by site --- */
	TEST("find by exact site returns result");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "site", "github.com", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_find_by_site(req, list), "should find");
	free_req(req);
	list_destroy(&list);

	TEST("find by partial site returns result");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "site", "github", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_find_by_site(req, list), "partial match should work");
	free_req(req);
	list_destroy(&list);

	TEST("find by non-existent site returns false");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "site", "notexist.xyz", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_find_by_site(req, list), "nothing should match");
	free_req(req);
	list_destroy(&list);

	/* --- find by email --- */
	TEST("find by exact email returns result");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "email", "alice@example.com", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_find_by_email(req, list), "should find");
	free_req(req);

	TEST("find by partial email returns result");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "email", "alice", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_find_by_email(req, list), "partial match should work");
	free_req(req);
	list_destroy(&list);

	TEST("find by non-existent email returns false");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "email", "ghost@nowhere.com", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_find_by_email(req, list), "nothing should match");
	free_req(req);
	list_destroy(&list);

	/* --- find by id --- */
	TEST("find by id=1 returns result");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "id", "1", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_find_by_id(req, list), "id 1 should exist");
	free_req(req);

	TEST("find by id returns correct site");
	iter = list_getiter(list);
	entry = iter_next(iter);
	EXPECT_STR_EQ(entry->site, "github.com", "site should be github.com");
	list_destroy(&list);

	TEST("find by non-existent id returns false");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "id", "9999", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_find_by_id(req, list), "id 9999 should not exist");
	free_req(req);
	list_destroy(&list);

	/* --- SQL injection via find --- */
	TEST("SQL injection in find site is safe");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "site", "' OR '1'='1", NULL, NULL, NULL, NULL, NULL, NULL);
	/* Either validator rejects it or it returns 0 rows — table must survive */
	pwdman_find_by_site(req, list);
	free_req(req);
	EXPECT_TRUE(db_count() > 0, "table must survive injection attempt");
	list_destroy(&list);

	TEST("SQL injection in find email is safe");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "email", "' OR '1'='1", NULL, NULL, NULL, NULL, NULL, NULL);
	pwdman_find_by_email(req, list);
	free_req(req);
	EXPECT_TRUE(db_count() > 0, "table must survive injection attempt");
	list_destroy(&list);
}

/* -------------------------------------------------------------------------
 * Suite: list (print all)
 * ---------------------------------------------------------------------- */

static void suite_list(void)
{
	SECTION("List (print all)");

	TEST("list returns non-NULL");
	List *all = pwdman_print_all();
	EXPECT_TRUE(all != NULL, "should not be NULL");

	TEST("list returns all records");
	EXPECT_TRUE((int)all->size >= 3, "should have at least 3 records");

	TEST("list entries have non-NULL fields");
	Iter *iter = list_getiter(all);
	struct pwdman *entry;
	bool fields_ok = true;
	while ((entry = iter_next(iter)) != NULL)
	{
		if (!entry->site || !entry->email || !entry->password)
		{
			fields_ok = false;
			break;
		}
	}
	EXPECT_TRUE(fields_ok, "all entries should have site, email, password");

	TEST("list entries have valid IDs");
	iter = list_getiter(all);
	bool ids_ok = true;
	while ((entry = iter_next(iter)) != NULL)
	{
		if (entry->id <= 0)
		{
			ids_ok = false;
			break;
		}
	}
	EXPECT_TRUE(ids_ok, "all IDs should be positive");

	list_destroy(&all);
}

/* -------------------------------------------------------------------------
 * Suite: update
 * ---------------------------------------------------------------------- */

static void suite_update(void)
{
	SECTION("Update");

	struct request *req;
	List *list;
	struct pwdman *entry;
	Iter *iter;

	TEST("update existing entry succeeds");
	req = make_req("/update",
		"id", "1",
		"site", "github.com",
		"email", "alice-updated@example.com",
		"password", "newpassword123"
	);
	EXPECT_TRUE(pwdman_update(req), "update should return true");
	free_req(req);

	TEST("updated email is persisted correctly");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "id", "1", NULL, NULL, NULL, NULL, NULL, NULL);
	pwdman_find_by_id(req, list);
	free_req(req);
	iter = list_getiter(list);
	entry = iter_next(iter);
	EXPECT_STR_EQ(entry->email, "alice-updated@example.com", "email should be updated");
	list_destroy(&list);

	TEST("updated password decrypts correctly");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "id", "1", NULL, NULL, NULL, NULL, NULL, NULL);
	pwdman_find_by_id(req, list);
	free_req(req);
	iter = list_getiter(list);
	entry = iter_next(iter);
	EXPECT_STR_EQ(entry->password, "newpassword123", "decrypted password should match");
	list_destroy(&list);

	/* SQL injection via update */
	TEST("SQL injection in update password is safe");
	int before = db_count();
	req = make_req("/update",
		"id", "1",
		"site", "github.com",
		"email", "alice@example.com",
		"password", "'; DROP TABLE info; --"
	);
	pwdman_update(req);
	free_req(req);
	EXPECT_TRUE(db_count() >= before, "table must survive injection in update");
}

/* -------------------------------------------------------------------------
 * Suite: delete
 * ---------------------------------------------------------------------- */

static void suite_delete(void)
{
	SECTION("Delete");

	struct request *req;

	TEST("delete existing entry succeeds");
	req = make_req("/delete", "id", "2", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_delete(req), "delete should return true");
	free_req(req);

	TEST("record is gone after delete");
	List *list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "id", "2", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_find_by_id(req, list), "id 2 should not exist anymore");
	free_req(req);
	list_destroy(&list);

	TEST("delete non-existent id returns false via count check");
	req = make_req("/delete", "id", "9999", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_count_by_id(req), "id 9999 should not exist");
	free_req(req);

	TEST("deleting same entry twice: second returns false via count check");
	req = make_req("/delete", "id", "2", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_count_by_id(req), "already deleted id should not exist");
	free_req(req);
}

/* -------------------------------------------------------------------------
 * Suite: crypto round-trip
 * ---------------------------------------------------------------------- */

static void suite_crypto(void)
{
	SECTION("Crypto round-trip");

	TEST("encrypt then decrypt returns original plaintext");
	char *ct = NULL, *iv = NULL, *tag = NULL, *plain = NULL;
	bool enc = crypto_encrypt("supersecret", &ct, &iv, &tag);
	EXPECT_TRUE(enc, "encrypt should succeed");

	TEST("decrypt produces original value");
	if (enc) {
		bool dec = crypto_decrypt(ct, iv, tag, &plain);
		if (dec) {
			EXPECT_STR_EQ(plain, "supersecret", "decrypted value should match");
			free(plain);
		} else {
			FAIL("decrypt returned false");
		}
		free(ct); free(iv); free(tag);
	} else {
		printf("  %-55s⏭  SKIP\n", "decrypt produces original value");
		tests_run--;
	}

	TEST("two encryptions of same plaintext produce different IVs");
	char *ct2 = NULL, *iv2 = NULL, *tag2 = NULL;
	ct = iv = tag = NULL;
	crypto_encrypt("samepassword", &ct, &iv, &tag);
	crypto_encrypt("samepassword", &ct2, &iv2, &tag2);
	bool ivs_differ = (strcmp(iv, iv2) != 0);
	EXPECT_TRUE(ivs_differ, "IVs should be random and different each time");
	free(ct);
	free(iv);
	free(tag);
	free(ct2);
	free(iv2);
	free(tag2);

	TEST("tampered ciphertext fails decrypt");
	ct = iv = tag = NULL;
	crypto_encrypt("original", &ct, &iv, &tag);
	/* flip first byte of ciphertext */
	ct[0] = (ct[0] == 'a') ? 'b' : 'a';
	bool bad_dec = crypto_decrypt(ct, iv, tag, &plain);
	EXPECT_FALSE(bad_dec, "tampered ciphertext should fail GCM tag check");
	if (bad_dec)
		free(plain);
	free(ct);
	free(iv);
	free(tag);

	TEST("tampered tag fails decrypt");
	ct = iv = tag = NULL;
	crypto_encrypt("original", &ct, &iv, &tag);
	tag[0] = (tag[0] == 'a') ? 'b' : 'a';
	bad_dec = crypto_decrypt(ct, iv, tag, &plain);
	EXPECT_FALSE(bad_dec, "tampered tag should fail GCM tag check");
	if (bad_dec)
		free(plain);
	free(ct);
	free(iv);
	free(tag);

	TEST("empty string encrypts and decrypts correctly");
	ct = iv = tag = plain = NULL;
	crypto_encrypt("", &ct, &iv, &tag);
	bool ok = crypto_decrypt(ct, iv, tag, &plain);
	EXPECT_TRUE(ok && strcmp(plain, "") == 0, "empty string round-trip should work");
	if (ok)
		free(plain);
	free(ct);
	free(iv);
	free(tag);

	TEST("long password encrypts and decrypts correctly");
	const char *long_pwd = "this-is-a-very-long-password-with-special-chars-!@#$%^&*()_+-=[]{}|;':\",./<>?";
	ct = iv = tag = plain = NULL;
	crypto_encrypt(long_pwd, &ct, &iv, &tag);
	ok = crypto_decrypt(ct, iv, tag, &plain);
	EXPECT_TRUE(ok && strcmp(plain, long_pwd) == 0, "long password round-trip should work");
	if (ok)
		free(plain);
	free(ct);
	free(iv);
	free(tag);
}

/* -------------------------------------------------------------------------
 * Suite: count_by_id
 * ---------------------------------------------------------------------- */

static void suite_count_by_id(void)
{
	SECTION("Count by ID");

	struct request *req;

	TEST("count_by_id returns true for existing id");
	req = make_req("/find", "id", "1", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_TRUE(pwdman_count_by_id(req), "id 1 should exist");
	free_req(req);

	TEST("count_by_id returns false for missing id");
	req = make_req("/find", "id", "9999", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_count_by_id(req), "id 9999 should not exist");
	free_req(req);

	TEST("count_by_id returns false for id 0");
	req = make_req("/find", "id", "0", NULL, NULL, NULL, NULL, NULL, NULL);
	EXPECT_FALSE(pwdman_count_by_id(req), "id 0 should not exist");
	free_req(req);
}

/* -------------------------------------------------------------------------
 * Suite: edge cases
 * ---------------------------------------------------------------------- */

static void suite_edge_cases(void)
{
	SECTION("Edge cases");

	struct request *req;

	TEST("add entry with special characters in password");
	req = make_req("/add",
		"site", "special.com",
		"email", "special@special.com",
		"password", "p@$$w0rd!#%^&*()",
		NULL, NULL
	);
	EXPECT_TRUE(pwdman_add(req), "special chars in password should work");
	free_req(req);

	TEST("special char password decrypts correctly");
	List *list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "site", "special.com", NULL, NULL, NULL, NULL, NULL, NULL);
	pwdman_find_by_site(req, list);
	free_req(req);
	Iter *iter = list_getiter(list);
	struct pwdman *entry = iter_next(iter);
	EXPECT_STR_EQ(entry->password, "p@$$w0rd!#%^&*()", "special chars should survive encrypt/decrypt");
	list_destroy(&list);

	TEST("add entry with unicode-like long site");
	req = make_req("/add",
		"site", "very-long-subdomain.example.co.uk",
		"email", "user@example.co.uk",
		"password", "password",
		NULL, NULL
	);
	EXPECT_TRUE(pwdman_add(req), "long site should be accepted");
	free_req(req);

	TEST("find on empty result returns false, not a crash");
	list = list_new(sizeof(struct pwdman), destroyer);
	req = make_req("/find", "site", "zzz-definitely-not-there.com", NULL, NULL, NULL, NULL, NULL, NULL);
	bool result = pwdman_find_by_site(req, list);
	EXPECT_FALSE(result, "should return false gracefully");
	free_req(req);
	list_destroy(&list);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

int main(void)
{
	printf("\n🔐 pwdman test suite\n");
	printf("════════════════════════════════════════════════════════════\n");

	/* Init crypto with a fixed test key so we don't need a passphrase prompt */
	unsigned char test_key[32] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
	};

	if (!crypto_init_with_key(test_key, sizeof(test_key)))
	{
		fprintf(stderr, "❌ Failed to initialise crypto for tests\n");
		return 1;
	}

	setup_test_db();

	suite_validator();
	suite_add();
	suite_find();
	suite_list();
	suite_update();
	suite_delete();
	suite_count_by_id();
	suite_crypto();
	suite_edge_cases();

	/* Cleanup test DB */
	unlink(".pwdman_test.db");

	printf("\n════════════════════════════════════════════════════════════\n");
	printf("Results: %d/%d passed", tests_passed, tests_run);
	if (tests_failed > 0)
		printf("  (%d failed ❌)\n", tests_failed);
	else
		printf("  — all good ✅\n");
	printf("\n");

	return tests_failed > 0 ? 1 : 0;
}