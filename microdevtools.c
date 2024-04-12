
#include "microdevtools.h"

#ifdef _TLS
#include "certs.h"
#endif

/*
 * COMMON
 */

int mdt_rand(int l, int h) {
  srand(time(NULL));
  return l < h ? (rand() % (h - l + 1)) + l : 0;
}

int mdt_is_empty(const char *s) {
  int rv = 1;
  if (s && (*s != '\0')) {
    apr_size_t l = strlen(s);
    for (apr_size_t i = 0; i < l; i ++) {
      // The string is not empty if it contains at least one non-empty character
      if (!apr_isspace(s[i])) {
        rv = 0;
        break;
      }
    }
  }
  return rv;
}

int mdt_is_int(const char *s) {
  int rv = 0;
  if (s && (*s != '\0')) {
    char *endp;
    (void)strtol(s, &endp, 10);
    rv = (endp != s) && (*endp == '\0');
  }
  return rv;
}

int mdt_is_double(const char *s) {
  int rv = 0;
  if (s && (*s != '\0')) {
    char *endp;
    (void)strtod(s, &endp);
    rv = (endp != s) && (*endp == '\0');
  }
  return rv;
}

int mdt_in_string(const char *s, const char *sub) {
  int rv = 0;
  if (s && sub) {
    apr_size_t ls, lsub;
    rv = ((ls = strlen(s)) > 0) && ((lsub = strlen(sub)) > 0) &&
         (lsub <= ls) && (strstr(s, sub) != NULL);
  }
  return rv;
}

// Allocates a string to a buffer of specified size
char *mdt_buffer(apr_pool_t *mp, const char *s, apr_size_t *bf_size) {
  char *result = NULL, *ends = NULL, str[(*bf_size)+1];
  if (mp && s && *bf_size > 0) {
    ends = apr_cpystrn(str, s, (*bf_size)+1);
  }
  if (ends) {
    *bf_size = ends - str;
    if ((*bf_size) > 0) {
      result = (char*) apr_palloc(mp, sizeof(char)*(*bf_size) + 1);
      if (result) {
        ends = apr_cpystrn(result, s, (*bf_size) + 1);
      }
    }
  }
  if (!result) {
    *bf_size = 0;
  }
  // The returned string always has a NULL terminator and a size of
  // at most bf_size-1 bytes
  return result;
}

char *mdt_str(apr_pool_t *mp, const char *s, apr_size_t sz) {
  char *result = NULL;
  if (mp && s && sz) {
    apr_size_t bf_size = sz;
    result = mdt_buffer(mp, s, &bf_size);
  }
  return result;
}

char *mdt_trim(apr_pool_t *mp, const char *s) {
  char *result = NULL;
  if (mp && s) {
    int start = 0, end = strlen(s) - 1;
    while (apr_isspace(s[start])) {
      start ++;
    }
    while ((end >= start) && apr_isspace(s[end])) {
      end --;
    }
    result = mdt_str(mp, s + start, end - start + 1);
  }
  return result;
}

// char *mdt_trim(apr_pool_t *pool, const char *str) {
//   int start = 0, end = strlen(str) - 1;
//   // Trova il primo carattere non vuoto dall'inizio della stringa
//   while (isspace((unsigned char)str[start])) {
//     start++;
//   }
//   // Trova l'ultimo carattere non vuoto dalla fine della stringa
//   while (end >= start && isspace((unsigned char)str[end])) {
//     end--;
//   }
//   // Alloca la memoria per la stringa trimmata
//   char *trimmed_str = apr_palloc(pool, end - start + 2);
//   // Copia i caratteri non vuoti nella nuova stringa
//   memcpy(trimmed_str, str + start, end - start + 1);
//   trimmed_str[end - start + 1] = '\0';
//   return trimmed_str;
// }

const char *mdt_strip_char(apr_pool_t *mp, const char *s, char c) {
  char *result = NULL;
  apr_size_t l, j = 0;
  if (mp && s) {
    l = (apr_size_t)strlen(s);
    if (l > 0) {
      result = (char*)apr_palloc(mp, sizeof(char) * (l + 1));
    }
  }
  if (result) {
    // Rebuilds the string with every element different from c
    for (apr_size_t i = 0; i < l; i ++) {
      if (s[i] != c) {
        result[j] = s[i];
        j ++;
      }
    }
    result[j] = '\0';
  }
  return !result ? s : (const char*)result;
}

char *mdt_slice(apr_pool_t *mp, const char *s, apr_size_t i, apr_size_t l) {
  char *result = NULL;
  apr_size_t len = 0;
  if (mp && s && (i >= 0) && (l > 0)) {
    len = (apr_size_t)strlen(s);
  }
  if ((len > 0) && (i <= (len - 1)) && (l <= (len - i))) {
    result = (char*)apr_palloc(mp, sizeof(char) * (l + 1));
  }
  if (result) {
    for (apr_size_t j = 0; j < l; j ++) {
      result[j] = s[i + j];
    }
    result[l] = '\0';
  }
  return result;
}

const char *mdt_str_replace(apr_pool_t *mp, const char *s, const char *f, const char *r) {
  char *result = NULL;
  int i = 0, cnt = 0, r_len = 0, f_len = 0;
  if (mp && s && f && r) {
    if ((*s != '\0') && (*f != '\0') && (*r != '\0')) {
      if (strcmp(f, r) != 0) {
        f_len = strlen(f);
        r_len = strlen(r);
      }
    }
  }
  if (f_len > 0 && r_len > 0) {
    for (i = 0; s[i] != '\0'; i++) {
      if (strstr(&s[i], f) == &s[i]) {
        cnt ++;
        i += f_len - 1;
      }
    }
  }
  if (cnt > 0) {
    result = (char*)apr_palloc(mp, i + cnt * (r_len-f_len) + 1);
  }
  if (result) {
    i = 0;
    while (*s) {
      if (strstr(s, f) == s) {
        strcpy(&result[i], r);
        i += r_len;
        s += f_len;
      } else {
        result[i++] = *s++;
      }
    }
    result[i] = '\0';
  }
  return !result ? s : (const char*)result;
}

const char *mdt_replace_char(apr_pool_t *mp, const char *s, char f, char r) {
  char *result = NULL;
  if (mp && s && f && r) {
    if((*s != '\0') && (f != r)) {
      result = apr_pstrdup(mp, s);
    }
  }
  if (result) {
    for (int i = 0; i < strlen(result); i++) {
      if (result[i] == f) {
        result[i] = r;
      }
    }
  }
  return !result ? s : (const char*)result;
}

char *mdt_empty_string_make(apr_pool_t *mp) {
  char *result = NULL;
  if (mp) {
    result = (char*)apr_palloc(mp, 1);
  }
  if (result) {
    result[0] = '\0';
  }
  return result;
}

apr_array_header_t* mdt_split(apr_pool_t *mp, const char *s, const char *sp)
{
  apr_array_header_t *result = NULL;
  char *str = NULL;
  const char *tmp = NULL;
  apr_size_t l_sp = 0;
  if (mp && s && sp) {
    if (strlen(s) > 0) {
      l_sp = (apr_size_t)strlen(sp);
    }
  }
  if(l_sp > 0) {
    result = apr_array_make(mp, 0, sizeof(const char*));
  }
  if (result) {
    str = apr_pstrdup(mp, s);
  }
  if (str) {
    char *ptr = strstr(str, sp);
    while (ptr) {
      *ptr = '\0';
      if (strlen(str) <= 0) {
        tmp = (const char*)mdt_empty_string_make(mp);
        if (tmp) {
          APR_ARRAY_PUSH(result, const char*) = tmp;
        }
      } else {
        tmp = apr_pstrdup(mp, str);
        if (tmp) {
          APR_ARRAY_PUSH(result, const char*) = tmp;
        }
      }
      str = ptr + l_sp;
      ptr = strstr(str, sp);
    }
  }
  if (strlen(str) <= 0) {
    tmp = (const char*)mdt_empty_string_make(mp);
    if (tmp) {
      APR_ARRAY_PUSH(result, const char*) = tmp;
    }
  } else {
    tmp = apr_pstrdup(mp, str);
    if (tmp) {
      APR_ARRAY_PUSH(result, const char*) = tmp;
    }
  }
  return result;
}

char *mdt_join(apr_pool_t *mp, apr_array_header_t *a, const char *sp)
{
  int valid_input = 0, valid_array = 0;
  apr_size_t sp_l;
  char *item, *result = NULL;
  apr_array_header_t *tmp = NULL;
  valid_input = mp && a;
  if (valid_input) {
    valid_array = a->nelts > 0;
  }
  if (valid_array) {
    if (!sp) {
      result = apr_array_pstrcat(mp, a, 0);
    } else {
      sp_l = (apr_size_t)strlen(sp);
      if (sp_l > 0) {
        for (int i = 0; i < a->nelts; i ++) {
          item = APR_ARRAY_IDX(a, i, char*);
          if (item) {
            if (!tmp) {
              tmp = apr_array_make(mp, a->nelts, sizeof(char*));
            }
          }
          if (tmp) {
            APR_ARRAY_PUSH(tmp, char*) = apr_pstrdup(mp, item);
            if (i < (a->nelts - 1)) {
              APR_ARRAY_PUSH(tmp, char*) = apr_pstrdup(mp, sp);
            }
          }
        }
      }
      if (tmp && (tmp->nelts > 0)) {
        result = apr_array_pstrcat(mp, tmp, 0);
      }
    }
  }
  return result;
}

char *mdt_md5(apr_pool_t *mp, const char *s)
{
  char *result = NULL;
  apr_size_t l = 0;
  unsigned char digest[APR_MD5_DIGESTSIZE];
  if (mp && s) { 
    l = strlen(s);
  }
  if(l > 0) {
    apr_md5_ctx_t ctx;
    apr_md5_init(&ctx);
    apr_md5_update(&ctx, s, l);
    apr_md5_final(digest, &ctx);
    result = (char*)apr_pcalloc(mp, 32 + 1);
  }
  if (result) {
    for (int i = 0; i < APR_MD5_DIGESTSIZE; i ++) {
      sprintf(&result[i * 2], "%02x", digest[i]);
    }
  }
  return result;
}

char *mdt_base64_encode(apr_pool_t *mp, const char *s)
{
  char *result = NULL;
  apr_size_t l = 0;
  if (mp && s) {
    l = (apr_size_t)strlen(s);
  }
  if (l > 0) {
    result = (char*)apr_pcalloc(mp, apr_base64_encode_len(l));
  }
  if (result != NULL) {
    apr_base64_encode(result, s, l);
  }
  return result;
}

char *mdt_base64_decode(apr_pool_t* mp, const char *s)
{
  char *result = NULL;
  apr_size_t s_len = 0, max_rv_len = 0, rv_len = 0;
  if (mp && s) {
    s_len = strlen(s);
  }
  if (s_len > 0) {
    max_rv_len = apr_base64_decode_len(s);
  }
  if (max_rv_len > 0) {
    result = (char*)apr_palloc(mp, max_rv_len);
  }
  if (result) {
    rv_len = apr_base64_decode(result, s);
  }
  if (rv_len >= 0) {
    result[rv_len] = '\0';
  }
  return result;
}

apr_table_t* mdt_args_to_table(apr_pool_t *mp, const char *q)
{
  apr_table_t *result = NULL;
  apr_array_header_t *args, *elts;
  args = mdt_split(mp, q, "&");
  if (args && args->nelts) {
    result = apr_table_make(mp, args->nelts);
    for (int i = 0; i < args->nelts; i++) {
      const char *arg = APR_ARRAY_IDX(args, i, const char*);
      elts = mdt_split(mp, arg, "=");
      if (elts && elts->nelts == 2) {
        apr_table_set(
          result,
          APR_ARRAY_IDX(elts, 0, const char*),
          APR_ARRAY_IDX(elts, 1, const char*)
        );
      }
    }
  }
  return result;
}

int mdt_table_nelts(apr_table_t *t)
{
  return t ? (apr_table_elts(t))->nelts : -1;
}

apr_table_entry_t* mdt_table_elt(apr_table_t *t, int i)
{
  apr_table_entry_t *result = NULL;
  if (t && (i >= 0)) {
    if (i < (apr_table_elts(t))->nelts) {
      result = &((apr_table_entry_t*)((apr_table_elts(t))->elts))[i];
    }
  }
  return result;
}

char *mdt_datetime(apr_pool_t *mp, apr_time_t t, const char *f)
{
  char *result = NULL;
  apr_time_exp_t tm;
  apr_size_t size = 100;
  const char *fm = NULL;
  char tmp[100] = {0};
  if (mp && t && f) {
    if (apr_time_exp_lt(&tm, t) == APR_SUCCESS) {
      fm = apr_pstrdup(mp, f);
      if (fm) {
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "Y", "%Y"), "y", "%y");
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "m", "%m"), "d", "%d");
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "H", "%H"), "h", "%I");
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "s", "%S"), "i", "%M");
      }
    }
  }
  if (fm) {
    if (apr_strftime(tmp, &size, 100, fm, &tm) == APR_SUCCESS) {
      result = apr_pstrdup(mp, tmp);
    }
  }
  return result;
}

char *mdt_datetime_local(apr_pool_t *mp, apr_time_t t, const char *f)
{
  char *result = NULL;
  apr_time_exp_t tm;
  apr_size_t size = 100;
  const char *fm = NULL;
  char tmp[100] = {0};
  if (mp && t && f) {
    if (apr_time_exp_lt(&tm, t) == APR_SUCCESS) {
      fm = apr_pstrdup(mp, f);
      if (fm) {
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "Y", "%Y"), "y", "%y");
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "m", "%m"), "d", "%d");
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "H", "%H"), "h", "%I");
        fm = mdt_str_replace(mp, mdt_str_replace(mp, fm, "s", "%S"), "i", "%M");
        fm = apr_pstrcat(mp, fm, "%z", NULL);
      }
    }
  }
  if (fm) {
    if (apr_strftime(tmp, &size, 100, fm, &tm) == APR_SUCCESS) {
      result = apr_pstrdup(mp, tmp);
    }
  }
  return result;
}

char *mdt_datetime_utc(apr_pool_t *mp, apr_time_t t, const char *f)
{
  apr_time_exp_t tm;
  apr_size_t size = 100;
  char tmp[100] = {0}, *result = NULL;
  if (mp && t) {
    // Usa apr_time_exp_gmt invece di apr_time_exp_lt
    if (apr_time_exp_gmt(&tm, t) == APR_SUCCESS) {
      // Formato desiderato
      const char *fm = "%Y-%m-%d %H:%M:%S";
      if (apr_strftime(tmp, &size, 100, fm, &tm) == APR_SUCCESS) {
        result = apr_pstrdup(mp, tmp);
      }
    }
  }
  return result;
}

int mdt_is_dir(const char *d, apr_pool_t *mp)
{
  apr_finfo_t finfo;
  return mp && d && (strlen(d) > 0) &&
    (apr_stat(&finfo, d, APR_FINFO_TYPE, mp) == APR_SUCCESS) &&
    (finfo.filetype == APR_DIR);
}

int mdt_is_file(const char *f, apr_pool_t *mp)
{
  apr_finfo_t finfo;
  return mp && f && (strlen(f) > 0) &&
    (apr_stat(&finfo, f, APR_FINFO_NORM, mp) == APR_SUCCESS);
}

apr_status_t mdt_file_open(apr_file_t **fd, const char *f, apr_int32_t fl, apr_pool_t *mp)
{
  apr_status_t result = APR_EGENERAL;
  if (mp && f) {
    result = apr_file_open(fd, f, fl, APR_OS_DEFAULT, mp);
  }
  return result;
}

apr_status_t mdt_file_open_read(apr_file_t **fd, const char *f, apr_pool_t *mp)
{
  return mdt_file_open(fd, f, APR_READ, mp);
}

apr_status_t mdt_file_open_append(apr_file_t **fd, const char *f, apr_pool_t *mp)
{
  return mdt_file_open(fd, f, APR_WRITE | APR_CREATE | APR_APPEND, mp);
}

apr_status_t mdt_file_open_truncate(apr_file_t **fd, const char *f,
                                   apr_pool_t *mp)
{
  return mdt_file_open(fd, f, APR_WRITE | APR_CREATE | APR_TRUNCATE, mp);
}

apr_size_t mdt_file_write(apr_file_t *fd, const char *buf, apr_size_t l)
{
  apr_size_t result = 0;
  if (fd && buf && (l > 0)) {
    apr_status_t st = apr_file_write_full(fd, buf, l, &result);
    if (st != APR_SUCCESS) {
      result = 0;
    }
  }
  return result;
}

apr_size_t mdt_file_read(apr_pool_t *mp, apr_file_t *fd, void **buf)
{
  apr_size_t result = 0;
  if (mp && fd && buf) {
    apr_finfo_t finfo;
    apr_status_t st = apr_file_info_get(&finfo, APR_FINFO_NORM, fd);
    apr_size_t fsize = 0;
    if (st == APR_SUCCESS) {
      fsize = (apr_size_t)finfo.size;
    }
    if (fsize <= 0) {
      *buf = NULL;
    } else {
      if (fsize > MDT_MAX_READ_BUFFER) {
        fsize = MDT_MAX_READ_BUFFER;
      }
      *buf = (void*)apr_palloc(mp, fsize);
      if (buf) {
        st = apr_file_read_full(fd, *buf, fsize, &result);
      }
    }
  }
  return result;
}

apr_status_t mdt_file_close(apr_file_t *fd)
{
  return apr_file_close(fd);
}

apr_time_t mdt_timestamp(int year, int month, int day, int hour,
                        int minute, int second)
{
  if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
    return apr_time_now();
  }
  if (year < 1970 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31 ||
    hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
    return MDT_ERROR_TIMESTAMP;
  }
  apr_time_exp_t timeExp;
  apr_time_t currentTime = apr_time_now(); // Ottieni il tempo corrente
  apr_time_exp_gmt(&timeExp, currentTime); // Inizializza la struttura con il tempo corrente
  timeExp.tm_year = year - 1900;  // Anno - 1900
  timeExp.tm_mon = month - 1;    // Mese (da 0 a 11)
  timeExp.tm_mday = day;         // Giorno del mese
  timeExp.tm_hour = hour;        // Ora del giorno
  timeExp.tm_min = minute;       // Minuto
  timeExp.tm_sec = second;       // Secondo
  timeExp.tm_usec = 0;           // Microsecondo
  apr_time_t unixTime;
  apr_time_exp_gmt_get(&unixTime, &timeExp);
  return unixTime;
}

apr_time_t mdt_now()
{
  return mdt_timestamp(0, 0, 0, 0, 0, 0);
}

apr_table_entry_t* mdt_table_entry(apr_table_t *t, int i)
{
  return (t != NULL) && (i >= 0) && (i < (apr_table_elts(t))->nelts)
    ? &((apr_table_entry_t*)((apr_table_elts(t))->elts))[i]
    : NULL;
}

// Legge i dati dallo standard input e li restituisce come una stringa.
// 'm' è il pool di memoria da utilizzare per l'allocazione di eventuali risorse.
char *mdt_pipein(apr_pool_t *mp)
{
  char *result = NULL;
  char buf[MDT_MAX_READ_BUFFER] = {0};
  apr_size_t l;
  apr_file_t *fd;
  apr_size_t bytes = MDT_MAX_READ_BUFFER - 1;
  apr_status_t st = apr_file_open_stdin(&fd, mp);
  if (st == APR_SUCCESS) {
    st = apr_file_read(fd, buf, &bytes);
  }
  if (st == APR_SUCCESS) {
    if (bytes > 0) {
      result = (char*)apr_palloc(mp, bytes + 1);
    }
    if (result) {
      memcpy(result, buf, bytes);
      result[bytes] = '\0';
    }
    apr_file_close(fd);
  }
  return result;
}

char *mdt_env(const char *e, apr_pool_t *mp)
{
  char *result;
  return mp && e && (apr_env_get(&result, e, mp) == APR_SUCCESS) ? result : NULL;
}

void mdt_daemonize()
{
  pid_t pid, sid;
  pid = fork();
  if (pid < 0) {
    perror("Fork failed");
    exit(1);
  }
  if (pid > 0) {
    exit(0);
  }
  sid = setsid();
  if (sid < 0) {
    perror("Error creating new session");
    exit(1);
  }
  pid = fork();
  if (pid < 0) {
    perror("Second fork failed");
    exit(1);
  }
  if (pid > 0) {
    exit(0);
  }
  if (chdir("/") < 0) {
    perror("Error changing working directory");
    exit(1);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

/*
 * JSON
 */

mdt_json_pair_t* mdt_json_pair_init(apr_pool_t *mp) {
  mdt_json_pair_t *result = NULL;
  if (mp != NULL) {
    if ((result = (mdt_json_pair_t*)apr_palloc(mp, sizeof(mdt_json_pair_t))) != NULL) {
      //result->pool = mp;
      result->key = NULL;
      result->val = NULL;
      result->type = MDT_JSON_T_ZERO;
    }
  }
  return result;
}

mdt_json_object_t* mdt_json_object_init(apr_pool_t *mp) {
  return (mdt_json_object_t*)apr_array_make(mp, 0, sizeof(mdt_json_pair_t*));
}

int mdt_json_object_add(apr_pool_t *mp, mdt_json_object_t *jo, mdt_json_type_t tp,
                       const char *k, void *v)
{
  int result = 0;
  mdt_json_pair_t *entry;
  if ((mp != NULL) && (jo != NULL) && (tp >= 0)) {
    if ((entry = mdt_json_pair_init(mp)) != NULL) {
      entry->key = k;
      entry->val = v;
      entry->type = tp;
      APR_ARRAY_PUSH(jo, mdt_json_pair_t*) = entry;
      result = 1;
    }
  }
  return result;
}

mdt_json_type_t mdt_int_type(apr_int64_t v)
{
  if (v < APR_INT32_MIN) {
    return MDT_JSON_T_INT64;
  } else if (v < APR_INT16_MIN) {
    return MDT_JSON_T_INT32;
  } else if (v <= APR_INT16_MAX) {
    return MDT_JSON_T_INT16;
  } else if (v <= APR_UINT16_MAX) {
    return MDT_JSON_T_UINT16;
  } else if (v <= APR_INT32_MAX) {
    return MDT_JSON_T_INT32;
  } else if (v <= APR_UINT32_MAX) {
    return MDT_JSON_T_UINT32;
  } else if (v < APR_INT64_MAX) {
    return MDT_JSON_T_INT64;
  } else {
    return MDT_JSON_T_ZERO;
  }
}

mdt_json_pair_t* mdt_json_array_entry_make(apr_pool_t *mp, int type,
                                         const char *key, json_object *val)
{
  mdt_json_pair_t *entry = mdt_json_pair_init(mp);
  entry->key = key != NULL ? apr_pstrdup(mp, key) : NULL;
  // Eseguo lo switch dei tipi predefiniti di json-c
  switch (type) {
    case json_type_null: {
      entry->type = MDT_JSON_T_NULL;
      entry->val = NULL;
    } break;
    case json_type_boolean:{
      entry->type = MDT_JSON_T_BOOLEAN;
      entry->val = (void*)apr_palloc(mp, sizeof(char));
      *((char*)entry->val) = json_object_get_boolean(val);
    } break;
    case json_type_double: {
      entry->type = MDT_JSON_T_DOUBLE;
      entry->val = (void*)apr_palloc(mp, sizeof(double));
      *((double*)entry->val) = json_object_get_double(val);
    } break;
    case json_type_int: {
      apr_uint64_t tmp_u = 0;
      apr_int64_t tmp_i = (apr_int64_t)json_object_get_int64(val);
      mdt_json_type_t int_type = mdt_int_type(tmp_i);
      if (!int_type) {
        tmp_u = (apr_uint64_t)json_object_get_uint64(val);
        if (tmp_u > APR_INT64_MAX) {
          int_type = MDT_JSON_T_UINT64;
        } else {
          int_type = MDT_JSON_T_INT64;
        }
      }
      if (int_type == MDT_JSON_T_INT16) {
        entry->type = MDT_JSON_T_INT16;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_int16_t));
        *((apr_int16_t*)entry->val) = (apr_int16_t)tmp_i;
      } else if (int_type == MDT_JSON_T_UINT16) {
        entry->type = MDT_JSON_T_UINT16;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_uint16_t));
        *((apr_uint16_t*)entry->val) = (apr_uint16_t)tmp_i;
      } else if (int_type == MDT_JSON_T_INT32) {
        entry->type = MDT_JSON_T_INT32;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_int32_t));
        *((apr_int32_t*)entry->val) = (apr_int32_t)tmp_i;
      } else if (int_type == MDT_JSON_T_UINT32) {
        entry->type = MDT_JSON_T_UINT32;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_uint32_t));
        *((apr_uint32_t*)entry->val) = (apr_uint32_t)tmp_i;
      } else if (int_type == MDT_JSON_T_INT64) {
        entry->type = MDT_JSON_T_INT64;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_int64_t));
        *((apr_int64_t*)entry->val) = (apr_int64_t)tmp_i;
      } else if (int_type == MDT_JSON_T_UINT64) {
        entry->type = MDT_JSON_T_UINT64;
        entry->val = (void*)apr_palloc(mp, sizeof(apr_uint64_t));
        *((apr_uint64_t*)entry->val) = (apr_uint64_t)tmp_u;
      }
    } break;
    case json_type_string: {
      entry->type = MDT_JSON_T_STRING;
      entry->val = (void*)apr_pstrdup(mp, (const char*)json_object_get_string(val));
    } break;
  }
  return entry;
}

apr_array_header_t* mdt_json_parse(apr_pool_t *mp, json_object *jobj);

apr_array_header_t* mdt_json_parse_array(apr_pool_t *mp, json_object *jarr)
{
  int jarr_l;
  enum json_type type;
  //, *jtmp; è stata sostituita dalla seguente riga:
  json_object *jval; 
  mdt_json_pair_t *entry;
  apr_array_header_t *res = NULL;
  jarr_l = json_object_array_length(jarr);
  for (int i = 0; i < jarr_l; i ++) {
    jval = json_object_array_get_idx(jarr, i);
    type = json_object_get_type(jval);
    if (type == json_type_array) {
      entry = mdt_json_pair_init(mp);
      entry->type = MDT_JSON_T_ARRAY;
      entry->key = NULL;
      entry->val = (void*)mdt_json_parse_array(mp, jval);
    } else if (type == json_type_object) {
      //entry = (mdt_json_pair_t*)apr_palloc(mp, sizeof(mdt_json_pair_t));
      entry = mdt_json_pair_init(mp);
      entry->type = MDT_JSON_T_OBJECT;
      entry->key = NULL;
      entry->val = (void*)mdt_json_parse(mp, jval);
    } else {
      entry = mdt_json_array_entry_make(mp, type, NULL, jval);
    }
    if (res == NULL) res = apr_array_make(mp, 0, sizeof(mdt_json_pair_t*));
    APR_ARRAY_PUSH(res, mdt_json_pair_t*) = entry;
  }
  return res;
}

apr_array_header_t* mdt_json_parse(apr_pool_t *mp, json_object *jobj)
{
  mdt_json_pair_t *entry = NULL;
  apr_array_header_t *res = NULL;
  enum json_type type;
  json_object *jtmp;
  json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    switch (type) {
      case json_type_object: {
        if (json_object_object_get_ex(jobj, key, &jtmp)) {
          entry = mdt_json_pair_init(mp);
          entry->type = MDT_JSON_T_OBJECT;
          entry->key = apr_pstrdup(mp, key);
          entry->val = (void*)mdt_json_parse(mp, jtmp);
        }
      } break;
      case json_type_array: {
        if (json_object_object_get_ex(jobj, key, &jtmp)) {
          entry = mdt_json_pair_init(mp);
          entry->type = MDT_JSON_T_ARRAY;
          entry->key = apr_pstrdup(mp, key);
          entry->val = (void*)mdt_json_parse_array(mp, jtmp);
        }
      } break;
      default: {
        entry = mdt_json_array_entry_make(mp, type, key, val);
      } break;
    }
    if (res == NULL) res = apr_array_make(mp, 0, sizeof(mdt_json_pair_t*));
    APR_ARRAY_PUSH(res, mdt_json_pair_t*) = entry;
  }
  return res;
}

apr_array_header_t* mdt_json_decode(apr_pool_t *mp, const char *s)
{
  json_object *jobj;
  apr_array_header_t* result;
  jobj = json_tokener_parse(s);
  result = mdt_json_parse(mp, jobj);
  json_object_put(jobj);
  return result;
}

const char *mdt_json_encode(apr_pool_t *mp, const void *v, mdt_json_type_t tp)
{
  int len;
  apr_table_entry_t *e;
  apr_table_t *t;
  mdt_json_pair_t *p;
  // Dichiaro 2 array temporanei
  apr_array_header_t *obj, *arr = NULL;
  // Inizializzo il valore di ritorno
  const char *result = NULL;
  // Verifico che la memoria sia allocata e il tipo di dato specificato
  if (mp != NULL && tp) {
    if (v == NULL || tp == MDT_JSON_T_NULL) {
      // Il dato è una primitiva NULL
      result = apr_pstrdup(mp, MDT_JSON_NULL_S);
    } else if (tp == MDT_JSON_T_BOOLEAN) {
      // Il dato è una primitiva booleana
      result = apr_pstrdup(mp, *(char*)v ? MDT_JSON_TRUE_S : MDT_JSON_FALSE_S);
    } else if (tp == MDT_JSON_T_INT16) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%hd", *((apr_int16_t*)v));
    } else if (tp == MDT_JSON_T_UINT16) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%hu", *((apr_uint16_t*)v));
    } else if (tp == MDT_JSON_T_INT32) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%d", *((apr_int32_t*)v));
    } else if (tp == MDT_JSON_T_UINT32) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%u", *((apr_uint32_t*)v));
    } else if (tp == MDT_JSON_T_INT64) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%" APR_INT64_T_FMT, *((apr_int64_t*)v));
    } else if (tp == MDT_JSON_T_UINT64) {
      // Il dato è una primitiva intera
      result = apr_psprintf(mp, "%" APR_UINT64_T_FMT, *((apr_uint64_t*)v));
    } else if (tp == MDT_JSON_T_DOUBLE) {
      // Il dato è una primitiva double
      result = apr_psprintf(mp, "%0.8lf", *(double*)v);
    } else if (tp == MDT_JSON_T_STRING) {
      // Il dato è una stringa
      result = apr_psprintf(mp, "\"%s\"", apr_pescape_echo(mp, (const char*)v, 1));
    } else if (tp == MDT_JSON_T_JSON) {
      // Il dato è una stringa JSON pre-codificata
      result = apr_psprintf(mp, "%s", (const char*)v);
    } else if (tp == MDT_JSON_T_TIMESTAMP) {
      // Il dato è un apr_time_t
      result = apr_psprintf(mp, "%" APR_TIME_T_FMT, (apr_time_t)v);
    } else if (tp > MDT_JSON_T_VECTOR) {
      // Il dato è un vettore di elementi di tipo (tp - MDT_JSON_T_VECTOR)
      // La funzione si aspetta un vettore di primitive o di stringhe
      int type = tp - MDT_JSON_T_VECTOR;
      // Un vettore è una struttura apr_array_header_t di dati dello stesso tipo
      obj = (apr_array_header_t*)v;
      // Verifico che la struttura non sia vuota
      if (obj->nelts > 0) {
        if (arr == NULL) {
          // Alloco un array temporaneo per gli elementi del vettore
          arr = apr_array_make(mp, 1, sizeof(const char*));
        }
        if (arr != NULL) {
          // Ripeto per ogni elemento del vettore
          for (int i = 0; i < obj->nelts; i ++) {
            switch (type) {
              case MDT_JSON_T_NULL: {
                // Aggiungo all'array temporaneo una stringa null
                APR_ARRAY_PUSH(arr, const char*) = apr_pstrdup(mp, MDT_JSON_NULL_S);
              } break;
              case MDT_JSON_T_BOOLEAN: {
                // Estraggo il intero
                int entry = APR_ARRAY_IDX(obj, i, int);
                // Aggiungo all'array temporaneo una stringa true o false
                APR_ARRAY_PUSH(arr, const char*) = apr_pstrdup(mp, entry ? MDT_JSON_TRUE_S : MDT_JSON_FALSE_S);
              } break;
              case MDT_JSON_T_INT16: {
                // Estraggo il valore intero
                apr_int16_t entry = APR_ARRAY_IDX(obj, i, apr_int16_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%hd", entry);
              } break;
              case MDT_JSON_T_UINT16: {
                // Estraggo il valore intero
                apr_uint16_t entry = APR_ARRAY_IDX(obj, i, apr_uint16_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%hu", entry);
              } break;
              case MDT_JSON_T_INT32: {
                // Estraggo il valore intero
                apr_int32_t entry = APR_ARRAY_IDX(obj, i, apr_int32_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%d", entry);
              } break;
              case MDT_JSON_T_UINT32: {
                // Estraggo il valore intero
                apr_uint32_t entry = APR_ARRAY_IDX(obj, i, apr_uint32_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%u", entry);
              } break;
              case MDT_JSON_T_INT64: {
                // Estraggo il valore intero
                apr_int64_t entry = APR_ARRAY_IDX(obj, i, apr_int64_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%" APR_INT64_T_FMT, entry);
              } break;
              case MDT_JSON_T_UINT64: {
                // Estraggo il valore intero
                apr_uint64_t entry = APR_ARRAY_IDX(obj, i, apr_uint64_t);
                // Aggiungo all'array temporaneo il valore intero
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%" APR_UINT64_T_FMT, entry);
              } break;
              case MDT_JSON_T_DOUBLE: {
                // Estraggo il valore double
                double entry = APR_ARRAY_IDX(obj, i, double);
                // Aggiungo all'array temporaneo il valore double
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%0.8lf", entry);
              } break;
              case MDT_JSON_T_STRING: {
                // Estraggo il valore stringa
                // ------------------------------------------------------------
                // FIXME: deve essere eseguito l'escape della stringa estratta
                //        prima che venga aggiunta all'array temporaneo
                // ------------------------------------------------------------
                const char *entry = APR_ARRAY_IDX(obj, i, const char*);
                // Aggiungo all'array temporaneo il valore stringa
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\"", apr_pescape_echo(mp, entry, 1));
              } break;
              case MDT_JSON_T_JSON: {
                const char *entry = APR_ARRAY_IDX(obj, i, const char*);
                // Aggiungo all'array temporaneo il valore stringa JSON
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%s", entry);
              } break;
              case MDT_JSON_T_TIMESTAMP: {
                // Estraggo il valore apr_time_t
                apr_time_t entry = APR_ARRAY_IDX(obj, i, apr_time_t);
                // Aggiungo all'array temporaneo il valore apr_time_t
                APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "%" APR_TIME_T_FMT, entry);
              } break;
              case MDT_JSON_T_TABLE: {
                apr_table_t *entry = APR_ARRAY_IDX(obj, i, apr_table_t*);
                APR_ARRAY_PUSH(arr, const char*) =
                  //apr_psprintf(mp, "\"%s\"", apr_pescape_echo(mp, entry, 1));
                  mdt_json_encode(mp, (const void*)entry, MDT_JSON_T_TABLE);
              } break;
            }
          }
          // Al termine del ciclo for se l'array temporaneo non è vuoto
          // setto il valore di ritorno con la sua versione serializzata
          // in caso contrario il valore di ritorno contiene ancora NULL
          if (arr->nelts > 0) {
            const char *tmp_s = mdt_join(mp, arr, ",");
            if (tmp_s != NULL) {
              result = apr_psprintf(mp, "[%s]", tmp_s);
            }
            // @todo else
          }
        }
      }
    } else if (tp == MDT_JSON_T_TABLE) {
      t = (apr_table_t*)v;
      if (t && (len = (apr_table_elts(t))->nelts)) {
        if ((arr = apr_array_make(mp, len, sizeof(const char*)))) {
          for (int i = 0; i < len; i ++) {
            if ((e = &((apr_table_entry_t*)((apr_table_elts(t))->elts))[i])) {
              APR_ARRAY_PUSH(arr, const char*) =
                apr_psprintf(mp, "\"%s\":\"%s\"", (const char*)e->key,
                             apr_pescape_echo(mp, (const char*)e->val, 1));
            }
          }
          if (arr->nelts > 0) {
            const char *tmp_s = mdt_join(mp, arr, ",");
            if (tmp_s != NULL) {
              result = apr_psprintf(mp, "{%s}", tmp_s);
            }
          }
        }
      }
    } else if (tp == MDT_JSON_T_OBJECT) {
      // Il dato è un oggetto (ovvero un array associativo)
      // Un oggetto è una struttura apr_array_header_t di mdt_json_pair_t
      // La struttura mdt_json_pair_t contiene informazioni anche sul tipo di dato
      // La funzione richiede che le chiavi dei pair dell'array non siano NULL
      // altrimenti l'elemento non verrà aggiunto all'array temporaneo
      obj = (apr_array_header_t*)v;
      // Verifico che l'oggetto non sia vuoto
      if (obj->nelts > 0) {
        // Alloco un array temporaneo per gli elementi dell'oggetto
        if ((arr = apr_array_make(mp, 1, sizeof(const char*))) != NULL) {
          // Ripeto per ogni elemento dell'oggetto
          for (int i = 0; i < obj->nelts; i++) {
            // Estraggo il prossimo pair
            if ((p = APR_ARRAY_IDX(obj, i, mdt_json_pair_t*)) != NULL) {
              if (!p->key) continue;
              switch (p->type) {
                case MDT_JSON_T_NULL: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore null
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, MDT_JSON_NULL_S);
                } break;
                case MDT_JSON_T_BOOLEAN: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore boolean
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, *(char*)p->val ? MDT_JSON_TRUE_S : MDT_JSON_FALSE_S);
                } break;
                case MDT_JSON_T_INT16: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%hd", p->key, *(apr_int16_t*)p->val);
                } break;
                case MDT_JSON_T_UINT16: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%hu", p->key, *(apr_uint16_t*)p->val);
                } break;
                case MDT_JSON_T_INT32: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%d", p->key, *(apr_int32_t*)p->val);
                } break;
                case MDT_JSON_T_UINT32: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%u", p->key, *(apr_uint32_t*)p->val);
                } break;
                case MDT_JSON_T_INT64: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%" APR_INT64_T_FMT, p->key, *(apr_int64_t*)p->val);
                } break;
                case MDT_JSON_T_UINT64: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore integer
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%" APR_UINT64_T_FMT, p->key, *(apr_uint64_t*)p->val);
                } break;
                case MDT_JSON_T_TIMESTAMP: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore timestamp
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%" APR_TIME_T_FMT, p->key, *(apr_time_t*)p->val);
                } break;
                case MDT_JSON_T_DOUBLE: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore double
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%0.8lf", p->key, *(double*)p->val);
                } break;
                case MDT_JSON_T_STRING: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore string
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":\"%s\"", p->key, apr_pescape_echo(mp, (const char*)p->val, 1));
                } break;
                case MDT_JSON_T_JSON: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore string JSON
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, (const char*)p->val);
                } break;
                case MDT_JSON_T_OBJECT: {
                  // Aggiungo all'array temporaneo una coppia chiave/valore object
                  APR_ARRAY_PUSH(arr, const char*) = apr_psprintf(mp, "\"%s\":%s", p->key, mdt_json_encode(mp, p->val, MDT_JSON_T_OBJECT));
                } break;
                default: break;
              }
            }
          }
          if (arr->nelts > 0) {
            const char *tmp_s = mdt_join(mp, arr, ",");
            if (tmp_s != NULL) {
              result = apr_psprintf(mp, "{%s}", tmp_s);
            }
          }
        }
      }
    }
  }
  return result;
}

/*
 * LOGGER
 */

mdt_logger_t* mdt_log_alloc(apr_pool_t *mp, apr_proc_mutex_t *m, const char *f,
                          apr_size_t sz)
{
  mdt_logger_t *result = (mdt_logger_t*)apr_palloc(mp, sizeof(mdt_logger_t));
  if (result != NULL) {
    result->pool = mp;
    result->fname = f;
    result->mutex = m;
    result->max_size = sz ? sz : MDT_LOG_MAX_FILE_SIZE;
    apr_status_t st = mdt_file_open_append(&(result->fh), f, mp);
    if (st != APR_SUCCESS) {
      return NULL;
    }
    mdt_log_rotate(result);
  }
  return result;
}

void mdt_log_rotate(mdt_logger_t *l)
{
  apr_finfo_t finfo;
  // Estraggo i metadati del file di log corrente
  apr_status_t rv = apr_file_info_get(&finfo, APR_FINFO_SIZE, l->fh);
  if (rv != APR_SUCCESS) {
    return;
  }
  // Estraggo la dimensione del file di log corrente
  apr_off_t sz = finfo.size;
  // Se la dimensione del file corrente è inferiore a quella massima termino
  if (sz < l->max_size) {
    return;
  }
  // Genero un nome di file per il file di log originale
  // con il timestamp unix corrente per non sovrascrivere file precedenti
  apr_time_t ts = mdt_now();
  if (ts <= 0) {
    return;
  }
  const char *ts_s = apr_psprintf(l->pool, "%" APR_INT64_T_FMT, ts);
  if (ts_s == NULL) {
    return;
  }
  char *fname_old = apr_psprintf(l->pool, "%s_%s.old", l->fname, ts_s);
  if (fname_old == NULL) {
    return;
  }
  // Rinomino il file l->fname in fname_old
  // l->fh adesso punta al file fname_old pertanto le operazioni di
  // scrittura vengono registrate ancora sul file originale rinominato
  rv = apr_file_rename(l->fname, fname_old, l->pool);
  if (rv != APR_SUCCESS) {
    return;
  }
  // Apro un nuovo file con il nome l->fname
  // fh_new e l->fh non puntano allo stesso file
  apr_file_t *fh_new;
  rv = mdt_file_open_truncate(&fh_new, l->fname, l->pool);
  if (rv != APR_SUCCESS) {
    // Provo a ripristinare il nome del file di ol originale
    apr_file_rename(fname_old, l->fname, l->pool);
    return;
  }
  // Scrivo '--log-rotate' sul file originale ancora puntato da l->fh
  int w_size = apr_file_printf(l->fh, "--log-rotate\r\n");
  if (w_size <= 0) {
    // Provo a ripristinare il nome del file di ol originale
    apr_file_rename(fname_old, l->fname, l->pool);
    return;
  }
  // Copio il descrittore di fh_new in l->fh
  // Da questo momento le oprazioni di scrittura usano sul nuovo file
  // l->fh e fh_new contengono 2 copie dello stesso descrittore
  rv = apr_file_dup2(l->fh, fh_new, l->pool);
  if (rv != APR_SUCCESS) {
    // Provo a ripristinare il nome del file di ol originale
    apr_file_rename(fname_old, l->fname, l->pool);
    return;
  }
  // Chiudo la copia del descrittore di file in fh_new
  apr_file_close(fh_new);
}

void mdt_log_destroy(mdt_logger_t *l)
{
  if (l != NULL) {
    if (l->fh != NULL) {
      apr_file_close(l->fh);
      l->fh = NULL;
    }
    l = NULL;
  }
}

/*
 * DBD
 */

mdt_dbd_t* mdt_dbd_alloc(apr_pool_t *mp)
{
  mdt_dbd_t *result = NULL;
  if (mp != NULL) {
    if ((result = (mdt_dbd_t*)apr_palloc(mp, sizeof(mdt_dbd_t))) != NULL) {
      result->drv = NULL;
      result->hdl = NULL;
      result->er_msg = NULL;
      result->trx = NULL;
      result->err = 0;
    }
  }
  return result;
}

int mdt_dbd_open(apr_pool_t *mp, mdt_dbd_t *d, const char *s, const char *c)
{
  int result = 0;
  apr_status_t rv;
  d->er_msg = NULL;
  d->drv = NULL;
  d->hdl = NULL;
  d->err = 0;
  if (mp && d) {
    rv = apr_dbd_get_driver(mp, s, &(d->drv));
  }
  if (rv == APR_SUCCESS) {
    rv = apr_dbd_open_ex(d->drv, mp, c, &(d->hdl), &(d->er_msg));
  }
  result = rv == APR_SUCCESS;
  if (!result) {
    d->drv = NULL;
    d->hdl = NULL;
    d->err = 1;
  }
  return result;
}

const char *mdt_dbd_escape(apr_pool_t *mp, mdt_dbd_t *d, const char *s)
{
  return ((mp == NULL) || (d == NULL) || (s == NULL))
    ? NULL
    : apr_dbd_escape(d->drv, mp, s, d->hdl);
}

int mdt_dbd_query(apr_pool_t *mp, mdt_dbd_t *d, const char *sql)
{
  int result = 0;
  if (mp == NULL || d == NULL || sql == NULL) return -1;
  d->er_msg = NULL;
  d->err = apr_dbd_query(d->drv, d->hdl, &result, sql);
  if (d->err) {
    d->er_msg = apr_pstrdup(mp, apr_dbd_error(d->drv, d->hdl, d->err));
    return -1;
  }
  return result;
}

int mdt_dbd_transaction_start(apr_pool_t *mp, mdt_dbd_t *dbd)
{
  int rv = 1;
  const char *error;
  if ((mp != NULL) && (dbd != NULL)) {
    if ((rv = apr_dbd_transaction_start(dbd->drv, mp, dbd->hdl, &(dbd->trx)))) {
      if ((error = apr_dbd_error(dbd->drv, dbd->hdl, rv)) != NULL) {
        dbd->er_msg = apr_pstrdup(mp, error);
      }
    }
  }
  return (rv == 0 ? 0 : -1);
}

int mdt_dbd_transaction_end(apr_pool_t *mp, mdt_dbd_t *dbd)
{
  int rv = 1;
  const char *error;
  if ((mp != NULL) && (dbd != NULL)) {
    if ((rv = apr_dbd_transaction_end(dbd->drv, mp, dbd->trx))) {
      if ((error = apr_dbd_error(dbd->drv, dbd->hdl, rv)) != NULL) {
        dbd->er_msg = apr_pstrdup(mp, error);
      }
    }
  }
  return (rv == 0 ? 0 : -1);
}

apr_array_header_t* mdt_dbd_result_to_array(apr_pool_t *mp, mdt_dbd_t *dbd,
                                           apr_dbd_results_t *res)
{
  apr_table_t *rec;
  apr_dbd_row_t *row = NULL;
  apr_array_header_t *result = NULL;
  const char *k, *v;
  int rv, first_rec, num_fields;
  if ((mp != NULL) && (dbd != NULL) && (res != NULL)) {
    if ((rv = apr_dbd_get_row(dbd->drv, mp, res, &row, -1)) != -1) {
      first_rec = 1;
      while (rv != -1) {
        if (first_rec) {
          num_fields = apr_dbd_num_cols(dbd->drv, res);
          result = apr_array_make(mp, num_fields, sizeof(apr_table_t*));
          first_rec = 0;
        }
        rec = apr_table_make(mp, num_fields);
        for (int i = 0; i < num_fields; i++) {
          k = apr_dbd_get_name(dbd->drv, res, i);
          v = apr_dbd_get_entry(dbd->drv, row, i);
          apr_table_set(rec, apr_pstrdup(mp, k),
                        apr_pstrdup(mp, mdt_is_empty(v) ? "NULL" : v));
        }
        APR_ARRAY_PUSH(result, apr_table_t*) = rec;
        rv = apr_dbd_get_row(dbd->drv, mp, res, &row, -1);
      }
    }
  }
  return result;
}

int mdt_dbd_prepared_query(apr_pool_t *mp, mdt_dbd_t *dbd, const char *sql,
                          apr_table_t *args)
{
  apr_table_entry_t *arg;
  const char **args_ar, *err;
  apr_dbd_prepared_t *stmt = NULL;
  int result = 0, nelts, rv;
  if (mp != NULL && dbd != NULL && sql != NULL) {
    dbd->er_msg = NULL;
    if ((nelts = apr_table_elts(args)->nelts) > 0) {
      args_ar = (const char**)apr_palloc(mp, sizeof(const char*)*nelts);
      if (args_ar != NULL) {
        for (int i = 0; i < nelts; i++) {
          arg = mdt_table_entry(args, i);
          if (arg != NULL) {
            args_ar[i] = apr_pstrdup(mp, arg->val);
            if (args_ar[i] == NULL) {
              return -1;
            }
          }
        }
        dbd->err = apr_dbd_prepare(dbd->drv, mp, dbd->hdl, sql, NULL, &stmt);
        if (dbd->err) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, dbd->err);
          dbd->er_msg = apr_pstrdup(mp, err);
          return -1;
        }
        dbd->err = apr_dbd_pquery(dbd->drv, mp, dbd->hdl, &result, stmt, nelts,
                                  args_ar);
        if (dbd->err) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, dbd->err);
          dbd->er_msg = apr_psprintf(mp, "%s", err);
          return -1;
        }
      }
    }
  }
  return result;
}

// int mdt_dbd_prepared_query(apr_pool_t *mp, mdt_dbd_t *dbd,
//                           const char *sql, const char **args, int sz) {
  
//   const char *err;
//   apr_dbd_prepared_t *stmt = NULL;
//   int result = 0, rv;
//   if (mp != NULL && dbd != NULL && sql != NULL && args != NULL && sz > 0) {
//     dbd->er_msg = NULL;
//     rv = apr_dbd_prepare(dbd->drv, mp, dbd->hdl, sql, NULL, &stmt);
//     if (rv) {
//       err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
//       dbd->er_msg = apr_pstrdup(mp, err);
//       return -1;
//     }
//     rv = apr_dbd_pquery(dbd->drv, mp, dbd->hdl, &result, stmt, sz, args);
//     if (rv) {
//       err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
//       dbd->er_msg = apr_psprintf(mp, "%s", err);
//       return -1;
//     }
//   }
//   return result;
// }

apr_array_header_t* mdt_dbd_prepared_select(apr_pool_t *mp, mdt_dbd_t *dbd,
                                           const char *sql, apr_table_t *args)
{
  int rv, nelts;
  apr_dbd_results_t *res = NULL;
  apr_array_header_t *result = NULL;
  char **args_ar;
  const char *err;
  apr_table_entry_t *arg;
  apr_dbd_prepared_t *stmt = NULL;
  if ((mp != NULL) && (dbd != NULL) && (sql != NULL) && (args != NULL)) {
    if ((nelts = apr_table_elts(args)->nelts) > 0) {
      if ((args_ar = (char**)apr_palloc(mp, sizeof(char*)*nelts)) != NULL) {
        for (int i = 0; i < nelts; i++) {
          if ((arg = mdt_table_entry(args, i)) != NULL) {
            if ((args_ar[i] = apr_psprintf(mp, "%s", arg->val)) == NULL) {
              return NULL;
            }
          }
        }
        rv = apr_dbd_prepare(dbd->drv, mp, dbd->hdl, sql, NULL, &stmt);
        if (rv) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
          dbd->er_msg = apr_psprintf(mp, "%s", err);
          return NULL;
        }
        rv = apr_dbd_pselect(dbd->drv, mp, dbd->hdl, &res,
                             stmt, 0, nelts, (const char**)args_ar);
        if (rv) {
          err = apr_dbd_error(dbd->drv, dbd->hdl, rv);
          dbd->er_msg = apr_psprintf(mp, "%s", err);
          return NULL;
        }
        result = mdt_dbd_result_to_array(mp, dbd, res);
      }
    }
  }
  return result;
}

apr_array_header_t* mdt_dbd_select(apr_pool_t *mp, mdt_dbd_t *d, const char *sql)
{
  int rv, err;
  apr_dbd_results_t *res = NULL;
  apr_dbd_row_t *row = NULL;
  apr_array_header_t *result = NULL;
  apr_table_t *rec;
  const char *k, *v;
  int first_rec, num_fields;
  if ((mp != NULL) && (d != NULL) && (sql != NULL)) {
    d->er_msg = NULL;
    if ((err = apr_dbd_select(d->drv, mp, d->hdl, &res, sql, 0))) {
      d->er_msg = apr_pstrdup(mp, apr_dbd_error(d->drv, d->hdl, err));
    } else {
      if (res != NULL) {
        if ((rv = apr_dbd_get_row(d->drv, mp, res, &row, -1)) != -1) {
          result = NULL;
          first_rec = 1;
          while (rv != -1) {
            if (first_rec) {
              num_fields = apr_dbd_num_cols(d->drv, res);
              result = apr_array_make(mp, num_fields, sizeof(apr_table_t*));
              first_rec = 0;
            }
            rec = apr_table_make(mp, num_fields);
            for (int i = 0; i < num_fields; i++) {
              k = apr_dbd_get_name(d->drv, res, i);
              v = apr_dbd_get_entry(d->drv, row, i);
              apr_table_set(rec, apr_pstrdup(mp, k),
                            apr_pstrdup(mp, mdt_is_empty(v) ? "NULL" : v));
            }
            APR_ARRAY_PUSH(result, apr_table_t*) = rec;
            rv = apr_dbd_get_row(d->drv, mp, res, &row, -1);
          }
        }
      }
    }
  }
  return result;
}

int mdt_dbd_num_records(apr_array_header_t *r) {
  return (int)(r != NULL ? r->nelts : 0);
}

int mdt_dbd_num_columns(apr_array_header_t *r) {
  int result = 0;
  apr_table_t *rec;
  if (r && r->nelts) {
    if ((rec = APR_ARRAY_IDX(r, 0, apr_table_t*))) {
      result = apr_table_elts(rec)->nelts;
    }
  }
  return result;
}

apr_array_header_t* mdt_dbd_column_names(apr_pool_t *mp, apr_array_header_t *r)
{
  int nelts;
  apr_table_entry_t* e;
  apr_table_t *rec;
  apr_array_header_t *result = NULL;
  if (r != NULL && r->nelts) {
    if ((rec = APR_ARRAY_IDX(r, 0, apr_table_t*))) {
      if ((nelts = (apr_table_elts(rec)->nelts))) {
        if ((result = apr_array_make(mp, nelts, sizeof(const char*)))) {
          for (int i = 0; i < nelts; i++) {
            if ((e = &((apr_table_entry_t*)((apr_table_elts(rec))->elts))[i])) {
              APR_ARRAY_PUSH(result, const char*) = apr_pstrdup(mp, e->key);
            }
          }
        }
      }
    }
  }
  return result;
}

apr_table_t* mdt_dbd_record(apr_array_header_t *r, int i)
{
  return (r != NULL) && r->nelts && (i <= r->nelts-1)
    ? APR_ARRAY_IDX(r, i, apr_table_t*)
    : NULL;
}

const char *mdt_dbd_field_value(apr_array_header_t *res, int i, const char *k)
{
  if (res == NULL || res->nelts <= 0 || i > (res->nelts-1)) return NULL;
  apr_table_t* rec = APR_ARRAY_IDX(res, i, apr_table_t*);
  return apr_table_get(rec, k);
}

int mdt_dbd_field_set(apr_array_header_t *r, int i, const char *k, const char *v) {
  if (r == NULL || r->nelts <= 0 || i > (r->nelts-1)) return 1;
  apr_table_t* t = APR_ARRAY_IDX(r, i, apr_table_t*);
  apr_table_set(t, k, v);
  return 0;
}

int mdt_dbd_close(mdt_dbd_t *d) {
  return d == NULL ? 0 : apr_dbd_close(d->drv, d->hdl);
}

const char *mdt_dbd_driver_name(mdt_dbd_t *dbd) {
  return dbd == NULL ? NULL : apr_dbd_name(dbd->drv);
}

const char *mdt_dbd_error(mdt_dbd_t *d) {
  return (d == NULL) ? NULL : d->er_msg;
}

/*
 * HTTP REQUEST
 */

mdt_http_request_t* mdt_http_request_alloc(apr_pool_t *mp)
{
  mdt_http_request_t *result = NULL;
  if (mp) {
    result = (mdt_http_request_t*)apr_palloc(mp, sizeof(mdt_http_request_t));
    if (result) {
      result->pool = mp;
      result->args = NULL;
      result->body = NULL;
      result->headers = apr_table_make(mp, 0);
      result->parsed_uri = apr_table_make(mp, 0);
      result->query = NULL;
      result->uri = NULL;
      result->message = NULL;
      result->multipart_data = NULL;
      result->cookies = apr_table_make(mp, 0);
      result->username = NULL;
      result->password = NULL;
    }
  }
  return result;
}

apr_table_t *mdt_http_request_validate_args(mdt_http_request_t *r,
                                           mdt_request_validator_t *vd,
                                           int nargs) {
  int is_valid;
  const char *curr_v;
  apr_table_t *result = apr_table_make(r->pool, nargs);
  if (r && r->args) {
    for (int i = 0; i < nargs; ++i) {
      mdt_request_validator_t v = vd[i];
      curr_v = apr_table_get(r->args, v.key);
      if (curr_v == NULL) {
        continue;
      }
      is_valid = 0;
      if (v.type == MDT_REQUEST_T_INT) {
        is_valid = mdt_is_int(curr_v);
      } else if (v.type == MDT_REQUEST_T_DOUBLE) {
        is_valid = mdt_is_double(curr_v);
      } else if (v.type == MDT_REQUEST_T_STRING) {
        is_valid = !mdt_is_empty(curr_v);
      } else if (v.type == MDT_REQUEST_T_PASSWORD) {
        is_valid = !mdt_is_empty(curr_v);
      } else if (v.type == MDT_REQUEST_T_DATE) { // yyyy-mm-dd
        if (!mdt_is_empty(curr_v) && strlen(curr_v) == 10) {
          apr_array_header_t *curr_v_ar = mdt_split(r->pool, curr_v, "-");
          if (curr_v_ar && curr_v_ar->nelts == 3) {
            const char *y = APR_ARRAY_IDX(curr_v_ar, 0, const char*);
            const char *m = APR_ARRAY_IDX(curr_v_ar, 1, const char*);
            const char *d = APR_ARRAY_IDX(curr_v_ar, 2, const char*);
            is_valid = strlen(y) == 4 && mdt_is_int(y) &&
                       strlen(m) == 2 && mdt_is_int(m) &&
                       strlen(d) == 2 && mdt_is_int(d);
          }
        }
      }
      if (is_valid) {
        if (v.filter == MDT_REQUEST_F_MD5) {
          curr_v = mdt_md5(r->pool, curr_v);
        }
        apr_table_set(result, v.key, curr_v);
      }
    }
  }
  return result;
}

apr_table_t* mdt_http_request_validate_multipart_args(mdt_http_request_t *r,
                                                     mdt_request_validator_t *vd,
                                                     int nargs) {
  apr_table_t *result = NULL;
  if (r && vd && nargs) {
    int is_valid;
    const char *req_v;
    result = apr_table_make(r->pool, nargs);
    for (int i = 0; i < nargs; ++i) {
      mdt_request_validator_t v = vd[i];
      for (int j = 0; j < r->multipart_data->nelts; ++j) {
        apr_table_t *entry = APR_ARRAY_IDX(r->multipart_data, j, apr_table_t*);
        if (!entry) {
          continue;
        }
        const char *key = apr_table_get(entry, "name");
        if (!key || (strcmp(v.key, key) != 0)) {
          continue;
        }
        req_v = apr_table_get(entry, "value");
        if (!req_v) {
          continue;
        }
        is_valid = 0;
        if (v.type == MDT_REQUEST_T_INT) {
          is_valid = mdt_is_int(req_v);
        } else if (v.type == MDT_REQUEST_T_DOUBLE) {
          is_valid = mdt_is_double(req_v);
        } else if (v.type == MDT_REQUEST_T_STRING) {
          is_valid = !mdt_is_empty(req_v);
        } else if (v.type == MDT_REQUEST_T_PASSWORD) {
          is_valid = !mdt_is_empty(req_v);
        } else if (v.type == MDT_REQUEST_T_DATE) { // yyyy-mm-dd
          if (!mdt_is_empty(req_v) && strlen(req_v) == 10) {
            apr_array_header_t *req_v_ar = mdt_split(r->pool, req_v, "-");
            if (req_v_ar && req_v_ar->nelts == 3) {
              const char *y = APR_ARRAY_IDX(req_v_ar, 0, const char*);
              const char *m = APR_ARRAY_IDX(req_v_ar, 1, const char*);
              const char *d = APR_ARRAY_IDX(req_v_ar, 2, const char*);
              is_valid = strlen(y) == 4 && mdt_is_int(y) &&
                         strlen(m) == 2 && mdt_is_int(m) &&
                         strlen(d) == 2 && mdt_is_int(d);
            }
          }
        }
        if (is_valid) {
          if (v.filter == MDT_REQUEST_F_MD5) {
            req_v = mdt_md5(r->pool, req_v);
          }
          apr_table_set(result, v.key, req_v);
        }
      }
    }
  }
  return result;
}

/*
 * HTTP RESPONSE
 */

mdt_http_response_t* mdt_http_response_alloc(apr_pool_t *mp) {
  mdt_http_response_t *result = NULL;
  if (mp) {
    result = (mdt_http_response_t*)apr_palloc(mp, sizeof(mdt_http_response_t));
  }
  if (result) {
    result->pool = mp;
    result->headers = apr_table_make(mp, 0);
    result->status = 0;
    result->size = 0;
    result->buffer = NULL;
  }
  return result;
}

void mdt_http_response_hd_set(mdt_http_response_t *r, const char *k, const char *v) {
  if (r && k && v) {
    apr_table_set(r->headers, k, v);
  }
}

const char *mdt_http_response_hd_get(mdt_http_response_t *r, const char *k) {
  return r && k ? apr_table_get(r->headers, k) : NULL;
}

const char *mdt_http_response_hd_serialize(mdt_http_response_t *r) {
  const char *result = NULL;
  do {
    if (!r) break;
    int nelts = mdt_table_nelts(r->headers);
    if (nelts <= 0) break;
    apr_table_entry_t *e;
    e = mdt_table_entry(r->headers, 0);
    result = apr_psprintf(r->pool, "%s: %s\r\n", e->key, e->val);
    for (int i = 1; i < nelts; i++) {
      e = mdt_table_entry(r->headers, i);
      if (e) {
        char *h = apr_psprintf(r->pool, "%s: %s\r\n", e->key, e->val);
        result = apr_pstrcat(r->pool, result, h, NULL);
      }
    }
  } while(0);
  return result;
}

void mdt_http_response_buffer_set(mdt_http_response_t *r, void *buf, size_t sz) {
  if (r && buf && sz) {
    r->size = sz;
    r->buffer = apr_palloc(r->pool, sz);
    if (r->buffer) {
      memcpy(r->buffer, buf, sz);
    }
  }
}

/*
 * SERVICE
 */

mdt_service_t* mdt_alloc(apr_pool_t *mp) {
  mdt_service_t *result = NULL;
  if (mp) {
    result = (mdt_service_t*)apr_palloc(mp, sizeof(mdt_service_t));
  }
  if (result) {
    result->pool = mp;
    result->authorized = 0;
    result->er_msg = NULL;
    result->request = NULL;
    result->response = NULL;
    result->dbd = NULL;
    result->logger = NULL;
  }
  return result;
}

void mdt_route(mdt_service_t *s, const char *mth, const char *uri, mdt_route_t fn) {
  if (s && mth && uri && fn) {
    if (s->response && !s->response->status) {
      if (s->request) {
        if (s->request->method && strcmp(s->request->method, mth) == 0) {
          if (s->request->uri && strcmp(s->request->uri, uri) == 0) {
            s->response->status = fn(s);
          }
        }
      }
    }
  }
}

void mdt_printf(mdt_service_t *s, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  // Calcolo la lunghezza della stringa formattata
  apr_size_t len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (len > 0) {
    // Allocazione la memoria per la stringa formattata
    char *buffer = (char*)apr_palloc(s->pool, len + 1);
    if (buffer) {
      // Creo la stringa formattata
      va_start(args, fmt);
      vsnprintf(buffer, len + 1, fmt, args);
      va_end(args);
      buffer[len+1] = '\0';
      s->response->is_binary = 0;
      if (s->response->buffer == NULL) {
        // Inizializzo il body della response HTTP
        s->response->buffer = apr_pstrdup(s->pool, buffer);
      } else {
        // Concateno la stringa al body della response HTTP
        s->response->buffer = apr_pstrcat(s->pool, s->response->buffer, buffer, NULL);
      }
      if (s->response->buffer) {
        s->response->size = strlen(s->response->buffer);
      }
    }
  }
}

char *mdt_jwt_base64_encode(const unsigned char *s, int sz) {
  char *result = NULL;
  if (s && sz) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, s, sz);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);
    result = bufferPtr->data;
  }
  return result;
}

unsigned char *mdt_jwt_base64_decode(const char *s, int sz) {
  unsigned char *result = NULL;
  if (s && sz) {
    BIO *bio, *b64;
    result = (unsigned char*)malloc(sz);
    int decode_length = 0;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(s, sz);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decode_length = BIO_read(bio, result, sz);
    BIO_free_all(bio);
    // Trim padding characters '='
    while (result[decode_length - 1] == '=') {
      decode_length--;
    }
    result = realloc(result, decode_length + 1);
    result[decode_length] = '\0';
  }
  return result;
}

char *mdt_hmac_encode(const char *key, const char *s, apr_size_t sz) {
  char *result = NULL;
  if (key && s && sz) {
    unsigned int hmac_len;
    unsigned char hmac[EVP_MAX_MD_SIZE];
    HMAC(EVP_sha256(), key, strlen(key), (const unsigned char*)s, sz, hmac, &hmac_len);
    HMAC(EVP_sha256(), key, strlen(key), (const unsigned char*)s, sz, hmac, &hmac_len);
    result = mdt_jwt_base64_encode(hmac, hmac_len);
  }
  return result;
}

char *mdt_jwt_token_create(apr_pool_t *mp, apr_table_t *claims, const char *key) {
  char *result = NULL;
  const char *claims_str = NULL;
  char *enc_head = NULL, *enc_hmac = NULL, *enc_claims = NULL;
  const unsigned char head[] = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
  if (mp && claims && key) {
    claims_str = mdt_json_encode(mp, claims, MDT_JSON_T_TABLE);
  }
  if (claims_str) {
    enc_head = mdt_jwt_base64_encode(head, 27);
  }
  if (enc_head) {
    enc_claims = mdt_jwt_base64_encode((const unsigned char*)claims_str, strlen(claims_str));
  }
  if (enc_claims) {
    enc_hmac = mdt_hmac_encode(key, enc_claims, strlen(enc_claims));
  }
  if (enc_hmac) {
    result = apr_psprintf(mp, "%s.%s.%s", enc_head, enc_claims, enc_hmac);
    free(enc_hmac);
  }
  return result;
}

int mdt_jwt_token_validate(apr_pool_t *mp, const char *tok, const char *key) {
  int result = 0;
  const char *enc_claims = NULL, *enc_hmac = NULL, *gen_hmac = NULL;
  apr_array_header_t *tok_ar;
  if (mp && tok && key) {
    tok_ar = mdt_split(mp, tok, ".");
  }
  if (tok_ar && tok_ar->nelts == 3) {
    enc_claims = APR_ARRAY_IDX(tok_ar, 1, const char*);
  }
  if (enc_claims) {
    enc_hmac = APR_ARRAY_IDX(tok_ar, 2, const char*);
  }
  if (enc_hmac) {
    gen_hmac = (const char*)mdt_hmac_encode(key, enc_claims, strlen(enc_claims));
  }
  if (gen_hmac) {
    result = (int)(strcmp(enc_hmac, gen_hmac) == 0);
  }
  return result;
}


// -----------------------------------------------------------------------------

mdt_server_t* mdt_server_alloc(apr_pool_t *mp) {
  mdt_server_t *s;
  s = (mdt_server_t*)apr_palloc(mp, sizeof(mdt_server_t));
  if (s != NULL) {
    s->pool = mp;
    s->host = NULL;
    s->port = NULL;
    s->port_s = NULL;
    s->timeout = NULL;
    s->max_threads = NULL;
    s->log_file = NULL;
    s->logger = NULL;
    s->dbd_driver = NULL;
    s->dbd_conn_s = NULL;
    s->upload_dir = NULL;
    s->addr = NULL;
    s->addr_s = NULL;
  }
  return s;
}

void mdt_server_destroy(mdt_server_t *s) {
  if (s && s->pool) {
    if (s->logger) {
      if (s->logger->mutex) {
        apr_proc_mutex_destroy(s->logger->mutex);
      }
      if (s->logger->fh) {
        apr_file_close(s->logger->fh);
      }
    }
  }
}

int mdt_dbd_pool_alloc(apr_pool_t *mp) {
  dbd_pool = (mdt_dbd_pool_t*)apr_palloc(mp, sizeof(mdt_dbd_pool_t));
  return dbd_pool != NULL;
}

void mdt_dbd_pool_add(apr_pool_t *mp, const char *drv, const char *conn_s) {
  mdt_dbd_t *dbd = mdt_dbd_alloc(mp);
  if (dbd != NULL) {
    if (mdt_dbd_open(mp, dbd, drv, conn_s)) {
      APR_ARRAY_PUSH(dbd_pool->connections, mdt_dbd_t*) = dbd;
      dbd_pool->counter ++;
    }
  }
}

int mdt_dbd_pool_init(apr_pool_t *mp, const char *drv, const char *conn_s) {
  int result = 0;
  if (dbd_pool != NULL) {
    dbd_pool->counter = -1;
    dbd_pool->connections = apr_array_make(
      mp, MDT_DBD_POOL_INIT_SIZE, sizeof(mdt_dbd_t*));
    if (dbd_pool->connections != NULL) {
      apr_status_t rv;
      rv = apr_proc_mutex_create(
        &(dbd_pool->mutex), "dbd_pool_mutex", APR_LOCK_DEFAULT, mp);
      if (rv == APR_SUCCESS) {
        for (int i = 0; i < MDT_DBD_POOL_INIT_SIZE; ++i) {
          mdt_dbd_pool_add(mp, drv, conn_s);
        }
        result = dbd_pool->connections->nelts == MDT_DBD_POOL_INIT_SIZE;
      }
    }
  }
  return result;
}

mdt_dbd_t* mdt_dbd_pool_get() {
  mdt_dbd_t *dbd = NULL;
  apr_status_t rv = apr_proc_mutex_lock(dbd_pool->mutex);
  if (rv == APR_SUCCESS) {
    if (dbd_pool->connections->nelts > 0) {
      if (dbd_pool->counter >= 0) {
        dbd = APR_ARRAY_IDX(dbd_pool->connections, dbd_pool->counter, mdt_dbd_t*);
        dbd_pool->counter --;
      }
    }
    apr_proc_mutex_unlock(dbd_pool->mutex);
  }
  return dbd;
}

void mdt_dbd_pool_release() {
  if (dbd_pool != NULL && dbd_pool->connections != NULL) {
    apr_status_t rv = apr_proc_mutex_lock(dbd_pool->mutex);
    if (rv == APR_SUCCESS) {
      if (dbd_pool->connections->nelts > 0) {
        if (dbd_pool->counter < MDT_DBD_POOL_INIT_SIZE - 1) {
          dbd_pool->counter ++;
        }
      }
      apr_proc_mutex_unlock(dbd_pool->mutex);
    }
  }
}

void mdt_dbd_pool_destroy() {
  if (dbd_pool != NULL) {
    apr_status_t rv = apr_proc_mutex_lock(dbd_pool->mutex);
    if (rv == APR_SUCCESS) {
      if (dbd_pool->connections != NULL && dbd_pool->connections->nelts > 0) {
        for (int i = 0; i < MDT_DBD_POOL_INIT_SIZE; ++i) {
          mdt_dbd_t *dbd = APR_ARRAY_IDX(dbd_pool->connections, i, mdt_dbd_t*);
          if (dbd != NULL && dbd->drv && dbd->hdl) {
            mdt_dbd_close(dbd);
          }
        }
      }
      apr_proc_mutex_unlock(dbd_pool->mutex);
    }
  }
}

void mdt_http_request_headers_set(apr_pool_t *mp, mdt_http_request_t *rq, struct mg_http_message *hm) {
  apr_table_t *headers = NULL;
  size_t i, max = sizeof(hm->headers) / sizeof(hm->headers[0]);
  for (i = 0; i < max && hm->headers[i].name.len > 0; i++) {
    if (!(rq->headers)) {
      rq->headers = apr_table_make(mp, 1);
      if (!(rq->headers)) {
        break;
      }
    }
    struct mg_str *k, *v;
    const char *key = NULL, *val = NULL;
    k = &hm->headers[i].name;
    if (k) {
      key = apr_psprintf(mp, "%.*s", (int) k->len, k->ptr);  
    }
    v = &hm->headers[i].value;
    if (v) {
      val = apr_psprintf(mp, "%.*s", (int) v->len, v->ptr);
    }
    if (key && val) {
      apr_table_set(rq->headers, key, val);
    }
  }
}

apr_table_t* mdt_http_request_args_parse(apr_pool_t *mp, const char *s, const char *sp1, const char *sp2) {
  apr_table_t *result = NULL;
  if (mp && s && sp1 && sp2) {
    apr_array_header_t *ar = mdt_split(mp, s, sp1);
    if (ar && ar->nelts > 0) {
      for (int i = 0; i < ar->nelts; i++ ) {
        const char *entry = APR_ARRAY_IDX(ar, i, const char*);
        apr_array_header_t *pair = mdt_split(mp, entry, sp2);
        if (pair && pair->nelts > 0) {
          if (!result) {
            result = apr_table_make(mp, 1);
            if (!result) {
              return NULL;
            }
          }
          char *k, *v;
          char *trm_k, *trm_v;
          k = apr_pstrdup(mp, APR_ARRAY_IDX(pair, 0, const char*));
          v = apr_pstrdup(mp, APR_ARRAY_IDX(pair, 1, const char*));
          trm_k = mdt_trim(mp, k);
          trm_v = mdt_trim(mp, v);
          apr_table_set(result, trm_k, trm_v);
        }
      }
    }
  }
  return result;
}

apr_table_t* mdt_http_request_cookies_parse(apr_pool_t *mp, struct mg_http_message *hm) {
  apr_table_t *result = NULL;
  struct mg_str *cookies = mg_http_get_header(hm, "Cookie");
  if (cookies != NULL) {
    result = mdt_http_request_args_parse(mp, cookies->ptr, ";", "=");
  }
  return result;
}

apr_table_t* mdt_http_query_string_parse(apr_pool_t*mp, struct mg_http_message *hm) {
  apr_table_t *result = NULL;
  if (hm->query.len > 0) {
    const char *query = mdt_str(mp, hm->query.ptr, hm->query.len);
    result = mdt_http_request_args_parse(mp, query, "&", "=");
  }
  return result;
}

apr_table_t* mdt_http_request_body_parse(apr_pool_t*mp, struct mg_http_message *hm) {
  apr_table_t *result = NULL;
  if (hm->body.len > 0) {
    const char *body = mdt_str(mp, hm->body.ptr, hm->body.len);
    result = mdt_http_request_args_parse(mp, body, "&", "=");
  }
  return result;
}

// void mdt_signal_exit(int signum) {
//   if (signum == SIGTERM || signum == SIGINT) {
//     server_run = 0;
//   }
// }

void mdt_signal_handler(struct sigaction *sig_action, sighd_t cb) {
  sig_action->sa_handler = cb;
  sigemptyset(&sig_action->sa_mask);
  sig_action->sa_flags = 0;
  sigaction(SIGTERM, sig_action, NULL);
  sigaction(SIGINT, sig_action, NULL);
}

int mdt_http_request_multipart_parse(apr_pool_t *mp, mdt_http_request_t *rq, struct mg_connection *c, struct mg_http_message *hm) {
  char *err;
  apr_size_t fsize;
  const char *fname, *forig, *fpath;
  apr_table_t *entry;
  apr_file_t *fd;
  apr_status_t rv;
  struct mg_http_part part;
  size_t ofs = 0;
  rq->multipart_data = apr_array_make(mp, 0, sizeof(apr_table_t*));
  while ((ofs = mg_http_next_multipart(hm->body, ofs, &part)) != 0) {
    entry = apr_table_make(mp, 2);
    // Multipart files
    if ((int)part.filename.len > 0) {
      fname = apr_psprintf(mp, "%.*s", (int)part.name.len, part.name.ptr);
      forig = apr_psprintf(
        mp, "%.*s", (int)part.filename.len, part.filename.ptr);
      fsize = (apr_size_t)part.body.len;
      fpath = apr_psprintf(mp, "%s/%s", "/tmp", forig);
      rv = mdt_file_open_truncate(&fd, fpath, mp);
      if (rv == APR_SUCCESS) {
        fsize = mdt_file_write(fd, part.body.ptr, fsize);
        apr_table_set(entry, "file_name", fname);
        apr_table_set(entry, "file_path", fpath);
      }
    }
    // Multipart arguments
    else {
      char *key = apr_psprintf(mp, "%.*s", (int)part.name.len, part.name.ptr);
      apr_table_set(entry, "name", key);
      char *val = apr_psprintf(mp, "%.*s", (int)part.body.len, part.body.ptr);
      apr_table_set(entry, "value", val);
    }
    APR_ARRAY_PUSH(rq->multipart_data, apr_table_t*) = entry;
  }
  return rq->multipart_data->nelts > 0;
}

void mdt_http_request_handler(struct mg_connection *c, int ev, void *ev_data) {
  struct state_t {
    struct flag_t {
      int ev_data, fn_data, init, pool, logger, request, method, uri, query,
          body, multipart, response, handler, resp_headers, resp_size, dbd,
          dbd_handler, service;
    } flag;
    int error;
    apr_pool_t *pool;
    mdt_server_t *server;
    struct mg_http_message *hm;
    const char *er_msg;
  } st = {
    .flag.ev_data = 0, .flag.fn_data = 0, .flag.init = 0, .flag.pool = 0,
    .flag.logger = 0, .flag.request = 0, .flag.method = 0, .flag.uri = 0,
    .flag.query = 0, .flag.body = 0, .flag.multipart = 0, .flag.response = 0,
    .flag.resp_headers = 0, .flag.handler = 0, .flag.dbd = 0,
    .flag.dbd_handler = 0, .flag.service = 0, .flag.resp_size = 0, .error = 0,
    .server = NULL, .er_msg = NULL
  };
  do {
    if (ev == MG_EV_ACCEPT && c->fn_data != NULL) {
      #ifdef _TLS
      struct mg_tls_opts opts = {
        #ifdef _TLS_TWOWAY
        .ca = mg_str(s_tls_ca),
        #endif
        .cert = mg_str(s_tls_cert),
        .key = mg_str(s_tls_key)
      };
      mg_tls_init(c, &opts);
      #endif
    }
    if (ev == MG_EV_HTTP_MSG) {
      st.flag.ev_data = ev_data != NULL;
      if ((st.error = !st.flag.ev_data)) break;
      // Event data
      st.hm = (struct mg_http_message*)ev_data;
      st.flag.fn_data = c->fn_data != NULL;
      if ((st.error = !st.flag.fn_data)) break;
      // Server data
      st.server = (mdt_server_t*)c->fn_data;
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "Client connected");
      }
      {
        apr_status_t rv;
        // APR initialization
        rv = apr_initialize();
        st.flag.init = (rv == APR_SUCCESS);
        if ((st.error = !st.flag.init)) break;
        if (DEBUG) {
          mdt_log((st.server)->logger, "INFO", "Service APR initialized");
        }
        // Memory pool allocation
        rv = apr_pool_create(&st.pool, NULL);
        st.flag.pool = (rv == APR_SUCCESS);
        if ((st.error = !st.flag.pool)) break;
        if (DEBUG) {
          mdt_log((st.server)->logger, "INFO", "Service pool created");
        }
      }
      // Service allocation
      mdt_service_t* sv = mdt_alloc(st.pool);
      st.flag.service = sv != NULL;
      if ((st.error = !st.flag.service)) break;
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "Service data struct allocated");
      }
      // Logger
      sv->logger = (st.server)->logger;
      st.flag.logger = sv->logger != NULL;
      if ((st.error = !st.flag.logger)) break;
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "Service logger initialized");
      }
      // Request
      sv->request = mdt_http_request_alloc(st.pool);
      st.flag.request = sv->request != NULL;
      if ((st.error = !st.flag.request)) break;
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "Service HTTP request allocated");
      }
      // Request headers
      mdt_http_request_headers_set(st.pool, sv->request, st.hm);
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "HTTP request headers parsed");
      }
      // Request method
      st.flag.method = (st.hm)->method.len > 0;
      if ((st.error = !st.flag.method)) break;
      sv->request->method = mdt_str(
        st.pool, (st.hm)->method.ptr, (st.hm)->method.len);
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "HTTP request method parsed");
      }
      // Request URI
      st.flag.uri = (st.hm)->uri.len > 0;
      if ((st.error = !st.flag.uri)) break;
      sv->request->uri = mdt_str(st.pool, (st.hm)->uri.ptr, (st.hm)->uri.len);
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "HTTP request uri parsed");
      }
      // Request query string
      if (strcmp(sv->request->method, "GET") == 0) {
        if ((st.hm)->query.len) {
          sv->request->query = mdt_str(
            st.pool, (st.hm)->query.ptr, (st.hm)->query.len);
          st.flag.query = sv->request->query != NULL;
          if ((st.error = !st.flag.query)) break;
          sv->request->args = mdt_http_query_string_parse(sv->pool, st.hm);
          if (sv->request->args) {
            if (DEBUG) {
              mdt_log((st.server)->logger, "INFO", "HTTP query string parsed");
            }
          }
        } else {
          st.flag.query = 1;
        }
      }
      // Request body
      if ((st.hm)->body.len) {
        sv->request->body = mdt_str(st.pool, (st.hm)->body.ptr, (st.hm)->body.len);
        st.flag.body = sv->request->body != NULL;
        if ((st.error = !st.flag.body)) break;
        sv->request->args = mdt_http_request_body_parse(sv->pool, st.hm);
        if (sv->request->args) {
          if (DEBUG) {
            mdt_log((st.server)->logger, "INFO", "HTTP body parsed");
          }
        }
      } else {
        st.flag.body = 1;
      }
      {
        // Request multipart data
        const char *ctype;
        ctype = apr_table_get(sv->request->headers, "Content-Type");
        if (mdt_in_string(ctype, "multipart/form-data")) {
          st.flag.multipart =
            mdt_http_request_multipart_parse(st.pool, sv->request, c, st.hm);
          if ((st.error = !st.flag.multipart)) break;
          if (DEBUG) {
            mdt_log((st.server)->logger, "INFO", "HTTP request multipart parsed");
          }
        } else {
          st.flag.multipart = 1;
        }
      }
      // Authorization
      {
        char user[256] = {0}, pass[256] = {0};
        mg_http_creds(st.hm, user, sizeof(user), pass, sizeof(pass));
        if (strlen(user) > 0) {
          sv->request->username = apr_pstrdup(st.pool, user);
        }
        if (strlen(pass) > 0) {
          sv->request->password = apr_pstrdup(st.pool, pass);
        }
      }
      // Response
      sv->response = mdt_http_response_alloc(st.pool);
      st.flag.response = sv->response != NULL;
      if ((st.error = !st.flag.response)) break;
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "HTTP response allocated");
      }
      // Default response HTTP header Content-Type
      mdt_http_response_hd_set(sv->response, "Content-Type", "text/plain");
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "HTTP response Content-Type defined");
      }
      // DBD connection
      if ((st.server)->dbd_driver && (st.server)->dbd_conn_s) {
        if (DEBUG) {
          mdt_log((st.server)->logger, "INFO", "DBD connection configured");
        }
        sv->dbd = mdt_dbd_pool_get();
        st.flag.dbd = sv->dbd != NULL;
        if ((st.error = !st.flag.dbd)) break;
        if (DEBUG) {
          mdt_log((st.server)->logger, "INFO", "DBD connection opened");
        }
        st.flag.dbd_handler = sv->dbd->drv != NULL && sv->dbd->hdl != NULL;
        if ((st.error = !st.flag.dbd_handler)) break;
        if (DEBUG) {
          mdt_log((st.server)->logger, "INFO", "DBD handler initialized.");
        }
      }
      // ---------------
      // Service handler
      // ---------------
      mdt_handler(sv);
      st.er_msg = sv->er_msg;
      st.flag.handler = st.er_msg == NULL;
      if ((st.error = !st.flag.handler)) break;
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "Service handler executed.");
      }
      mdt_dbd_pool_release();
      if (DEBUG) {
        mdt_log((st.server)->logger, "INFO", "DBD connection released.");
      }
      // Response headers
      const char *http_hd;
      http_hd = mdt_http_response_hd_serialize(sv->response);
      st.flag.resp_headers = http_hd != NULL;
      if ((st.error = !st.flag.resp_headers)) break;
      // Default HTTP response status (404 - Not Found)
      if (sv->response->status == 0) {
        sv->response->status = 404;
      }
      if (sv->response->status != 200) {
        // Not 200 OK response
        const char *ctype;
        ctype = mdt_http_response_hd_get(sv->response, "Content-Type");
        if (ctype) {
          ctype = apr_psprintf(st.pool, "Content-Type: %s\r\n", ctype);
        }
        mg_http_reply(c, sv->response->status, ctype, "");
      } else {
        // 200 OK empty response
        st.flag.resp_size = sv->response->size > 0;
        if ((st.error = !st.flag.resp_size)) break;
        // 200 OK response
        mg_printf(c, HTTP_OK_FMT, http_hd ? http_hd : "", sv->response->size);
        if ((sv->response->size > 0) && sv->response->buffer) {
          mg_send(c, sv->response->buffer, sv->response->size);
        }
      }
    } /* ev == MG_EV_HTTP_MSG */
  } while (0);
  if (st.error) {
    const char ctype[] = "Content-Type: text/plain\r\n";
    if (!st.flag.ev_data) {
      mg_http_reply(c, 500, ctype, "Invalid event data.\r\n");
    } else if (!st.flag.fn_data) {
      mg_http_reply(c, 500, ctype, "Invalid server data.\r\n");
    } else if (!st.flag.init) {
      mg_http_reply(c, 500, ctype, "APR initialization error.\r\n");
    } else if (!st.flag.pool) {
      mg_http_reply(c, 500, ctype, "APR memory pool error.\r\n");
    } else if (!st.flag.service) {
      if (st.er_msg) {
        mg_http_reply(c, 500, ctype, st.er_msg);
      } else {
        mg_http_reply(c, 500, ctype, "HTTP service error.\r\n");
      }
    } else if (!st.flag.logger) {
      mg_http_reply(c, 500, ctype, "Invalid logger.\r\n");
    } else if (!st.flag.request) {
      mg_http_reply(c, 500, ctype, "Invalid request allocation.\r\n");
    } else if (!st.flag.method) {
      mg_http_reply(c, 500, ctype, "Invalid request method.\r\n");
    } else if (!st.flag.uri) {
      mg_http_reply(c, 500, ctype, "Invalid request uri.\r\n");
    } else if (!st.flag.query) {
      mg_http_reply(c, 500, ctype, "Invalid request query string.\r\n");
    } else if (!st.flag.body) {
      mg_http_reply(c, 500, ctype, "Invalid request body.\r\n");
    } else if (!st.flag.multipart) {
      mg_http_reply(c, 500, ctype, "Invalid request multipart data.\r\n");
    } else if (!st.flag.response) {
      mg_http_reply(c, 500, ctype, "Response allocation failure.\r\n");
    } else if (!st.flag.dbd) {
      mg_http_reply(c, 500, ctype, "DBD error.\r\n");
    } else if (!st.flag.dbd_handler) {
      mg_http_reply(c, 500, ctype, "DBD connection error.\r\n");
    } else if (!st.flag.handler) {
      if (st.er_msg) {
        mg_http_reply(c, 500, ctype, st.er_msg);
      } else {
        mg_http_reply(c, 500, ctype, "Request handler error.\r\n");
      }
    } else if (!st.flag.resp_headers) {
      mg_http_reply(c, 500, ctype, "Invalid HTTP response headers.\r\n");
    } else if (!st.flag.resp_size) {
      mg_http_reply(c, 500, ctype, "Empty response.\r\n");
    } else {
      mg_http_reply(c, 500, ctype, "General error.\r\n");
    }
  }
  if (st.flag.init) {
    if (st.flag.pool) {
      apr_pool_destroy(st.pool);
    }
    apr_terminate();
  }
}

int mdt_cmd_args_parse(mdt_server_t *s, int argc, char *argv[], char **er_msg) {
  struct state_t {
    struct flag_t {
      int input, arg_format, host, port, log_file;
    } flag;
    int error, result;
  } st = {
    .flag.input = 0, .flag.arg_format = 0, .error = 0, .result = 0
  };
  do {
    *er_msg = NULL;
    // Input validation
    st.flag.input = s != NULL && argv != NULL && argc > 1 && ((argc-1)%2) == 0;
    if ((st.error = !st.flag.input)) break;
    for (int i = 1; i < argc; i += 2) {
      // Command line arguments validation
      st.flag.arg_format = strlen(argv[i]) == 2;
      if ((st.error = !st.flag.arg_format)) break;
      // Command line arguments value
      if (argv[i][1] == 'h') {
        s->host = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'p') {
        s->port = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'P') {
        // https port
        s->port_s = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 't') {
        s->timeout = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'T') {
        s->max_threads = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'l') {
        s->log_file = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'd') {
        s->dbd_driver = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'D') {
        s->dbd_conn_s = apr_pstrdup(s->pool, argv[i+1]);
      } else if (argv[i][1] == 'u') {
        s->upload_dir = apr_pstrdup(s->pool, argv[i+1]);
      }
    }
    st.error = st.error || !(s->host) || !(s->port) || !(s->log_file);
    if (st.error) break;
    st.result = 1;
  } while (0);

  if (st.error) {
    if (!st.flag.input) {
      *er_msg = apr_pstrdup(s->pool, "Invalid input.");
    } else if (!st.flag.arg_format) {
      *er_msg = apr_pstrdup(s->pool, "Invalid arguments format.");
    } else if (s->host == NULL) {
      *er_msg = apr_pstrdup(s->pool, "Invalid host address.");
    } else if (s->port == NULL) {
      *er_msg = apr_pstrdup(s->pool, "Invalid port number.");
    } else if (s->log_file == NULL) {
      *er_msg = apr_pstrdup(s->pool, "Invalid log file.");
    } else {
      *er_msg = apr_pstrdup(s->pool, "General error.");
    }
  }
  return st.result;
}

int mdt_server_init(apr_pool_t *mp, mdt_server_t **s, int argc, char *argv[], char **er_msg) {
  struct state_t {
    struct flag_t {
      int input, args, addr, mutex, logger;
    } flag;
    int error, result;
    apr_status_t mutex;
    apr_proc_mutex_t *log_mutex;
  } st = {
    .flag.input = 0, .flag.args = 0, .flag.addr = 0, .flag.mutex = 0,
    .flag.logger = 0, .error = 0, .result = 0
  };
  do {
    *er_msg = NULL;
    st.flag.input = mp != NULL && *s != NULL && argv != NULL && argc > 1;
    if ((st.error = !st.flag.input)) break;
    st.flag.args = mdt_cmd_args_parse(*s, argc, argv, er_msg);
    if ((st.error = !st.flag.args)) break;
    (*s)->addr = apr_psprintf(mp, "%s:%s", (*s)->host, (*s)->port);
    if (TLS) {
      if ((*s)->port_s) {
        (*s)->addr_s = apr_psprintf(mp, "%s:%s", (*s)->host, (*s)->port_s);
      }
    }
    st.flag.addr = (*s)->addr != NULL;
    if ((st.error = !st.flag.addr)) break;
    apr_status_t rv;
    rv = apr_proc_mutex_create(&(st.log_mutex), "log_mutex", APR_LOCK_DEFAULT, mp);
    st.flag.mutex = rv == APR_SUCCESS;
    if ((st.error = !st.flag.mutex)) break;
    (*s)->logger = mdt_log_alloc(mp, st.log_mutex, (*s)->log_file, 0);
    st.flag.logger = (*s)->logger != NULL;
    if ((st.error = !st.flag.logger)) break;
    st.result = 1;
  } while (0);
  if (st.error) {
    if (!st.flag.input) {
      *er_msg = apr_pstrdup(mp, "Invalid input");
    } else if (!st.flag.args) {
      if (*er_msg == NULL) {
        *er_msg = apr_pstrdup(mp, "Invalid arguments");
      }
    } else if (!st.flag.args) {
      *er_msg = apr_pstrdup(mp, "Invalid address");
    } else if (!st.flag.mutex) {
      *er_msg = apr_pstrdup(mp, "Logger mutex initialization error");
    } else if (!st.flag.logger) {
      *er_msg = apr_pstrdup(mp, "Logger initialization error");
    } else {
      *er_msg = apr_pstrdup(mp, "General error");
    }
  }
  return st.result;
}



#ifdef _MDT_PDF

#define MDT_PDF_PADDING_W 6
#define MDT_PDF_PADDING_H 12

static void mdt_pdf_add_row(double *pos, double x, double y, double h) {
  pos[0] = x ? pos[0] + x : 10.0;
  pos[1] = y ? (pos[3] ? pos[1] + pos[3] + MDT_PDF_PADDING_H : pos[1] + y) : 30.0;
  pos[2] = h ? h : 30.0;
  if (pos[3]) pos[3] = 0;
}

static cairo_surface_t *z_pdf_scale_image(cairo_surface_t *s, double w, double h) {
  cairo_surface_t *res;
  cairo_t *cr;
  double ww, hh;
  res = cairo_surface_create_similar(s, cairo_surface_get_content(s), w, h);
  cr = cairo_create(res);
  ww = cairo_image_surface_get_width(s);
  hh = cairo_image_surface_get_height(s);
  cairo_scale(cr, w/ww, h/hh);
  cairo_set_source_surface(cr, s, 0, 0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_destroy(cr);
  return res;
}

static void mdt_pdf_set_col_border(cairo_t *cr, double *pos, double w, double size, const char *color) {
  cairo_set_line_width(cr, size);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
  cairo_set_source_rgb(cr, mdt_ctoi(color[0]), mdt_ctoi(color[1]), mdt_ctoi(color[2]));
  cairo_rectangle(cr, pos[0], pos[1], w, pos[2]);
  cairo_stroke(cr);
}

static void mdt_pdf_set_col_borders(cairo_t *cr, const char *border_pos, double *pos, double w) {
  if (border_pos[0] == '1') {
    cairo_move_to(cr, pos[0], pos[1]);
    cairo_line_to(cr, pos[0] + w, pos[1]);
  }
  if (border_pos[1] == '1') {
    cairo_move_to(cr, pos[0] + w, pos[1]);
    cairo_line_to(cr, pos[0] + w, pos[1]+pos[2]);
  }
  if (border_pos[2] == '1') {
    cairo_move_to(cr, pos[0], pos[1]+pos[2]);
    cairo_line_to(cr, pos[0] + w, pos[1]+pos[2]);
  }
  if (border_pos[3] == '1') {
    cairo_move_to(cr, pos[0], pos[1]);
    cairo_line_to(cr, pos[0], pos[1]+pos[2]);
  }
  cairo_stroke(cr);
}

static void mdt_pdf_col_fill_text(apr_pool_t*mp, cairo_t *cr, double *pos,
                          const char *color, const char *font, float size,
                          int wrap, const char *data, double *bord_y, int count) {
  apr_array_header_t *text_ar;
  char *text;
  double y;
  cairo_set_source_rgb(cr, mdt_ctoi(color[0]), mdt_ctoi(color[1]), mdt_ctoi(color[2]));
  cairo_select_font_face(cr, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, size);
  if (wrap) {
    text = (char*)apr_palloc(mp, strlen(data));
    for (int i = 0, count = 0; i < strlen(data); i++) {
      if ((count >= wrap - 1) && (data[i] == ' ')) {
        text[i] = '\r';
        count = 0;
      } else {
        text[i] = data[i];
        count ++;
      }
    }
    text_ar = mdt_split(mp, text, "\r");
  } else {
    text_ar = mdt_split(mp, data, "\r");
  }
  if (text_ar->nelts <= 1) {
    cairo_move_to(cr, pos[0] + MDT_PDF_PADDING_W, pos[1] + MDT_PDF_PADDING_H);
    cairo_show_text(cr, data);
  } else {
    y = pos[1] + MDT_PDF_PADDING_H;
    int pos_3 = 0;
    for (int i = 0; i < text_ar->nelts; i++) {
      cairo_move_to(cr, pos[0] + MDT_PDF_PADDING_W, y);
      cairo_show_text(cr, APR_ARRAY_IDX(text_ar, i, const char*));
      y += 10;
      pos_3 += 10;
    }
    if (pos_3) {

      pos[3] = pos_3;
      bord_y[count-1] = pos[3] + MDT_PDF_PADDING_H;
    }
  }
}

static void mdt_pdf_col_fill_image(cairo_t *cr, double *pos, const char *data, double w, double h) {
  cairo_surface_t *image, *scaled;
  image = cairo_image_surface_create_from_png(data);
  scaled = mdt_pdf_scale_image(image, w, h);
  cairo_set_source_surface(cr, scaled, pos[0], pos[1]);
  cairo_paint(cr);
  cairo_surface_destroy(image);
  cairo_surface_destroy(scaled);
}

static void mdt_pdf_add_col(apr_pool_t *mp, cairo_t *cr, double *pos, apr_array_header_t *meta, const char *data, double *bord_y, int count) {
  const char *col_tp, *col_w, *bg_color, *font_sz, *font_cl, *font_name, *text_wrap, *img_w, *img_h;
  int contains_text, contains_image;
  col_tp = APR_ARRAY_IDX(meta, 0, const char*);
  col_w = APR_ARRAY_IDX(meta, 1, const char*);
  bg_color = APR_ARRAY_IDX(meta, 6, const char*);
  font_sz = APR_ARRAY_IDX(meta, 7, const char*);
  font_cl = APR_ARRAY_IDX(meta, 8, const char*);
  font_name = APR_ARRAY_IDX(meta, 9, const char*);
  text_wrap = APR_ARRAY_IDX(meta, 10, const char*);
  img_w = APR_ARRAY_IDX(meta, 11, const char*);
  img_h = APR_ARRAY_IDX(meta, 12, const char*);
  contains_text = col_tp[0] == 't';
  contains_image = col_tp[0] == 'i';
  if (contains_text) {
    mdt_pdf_col_fill_text(mp, cr, pos, font_cl, font_name,
                         atof(font_sz), atoi(text_wrap), data, bord_y, count);
  } else if (contains_image) {
    mdt_pdf_col_fill_image(cr, pos, data, (double)atof(img_w), (double)atof(img_h));
  }
  pos[0] = pos[0] + atof(col_w);
}

static void mdt_pdf_add_col_border(apr_pool_t *mp, cairo_t *cr, double *pos, apr_array_header_t *meta) {
  const char *col_w, *border_pos, *border_sz, *border_tp, *border_cl;
  int have_all_borders;
  col_w = APR_ARRAY_IDX(meta, 1, const char*);
  border_pos = APR_ARRAY_IDX(meta, 2, const char*);
  border_sz = APR_ARRAY_IDX(meta, 3, const char*);
  border_tp = APR_ARRAY_IDX(meta, 4, const char*);
  border_cl = APR_ARRAY_IDX(meta, 5, const char*);
  have_all_borders = strcmp(border_pos, "1111") == 0;
  if (!have_all_borders) {
    mdt_pdf_set_col_borders(cr, border_pos, pos, atof(col_w));
  } else {
    mdt_pdf_set_col_border(cr, pos, atof(col_w), atof(border_sz), border_cl);
  }
  pos[0] = pos[0] + atof(col_w);
}

/** @example  const char *buffer;
              mdt_file_read(mp, "pdf_file.txt", &buffer);
              mdt_pdf_create(mp, buffer, "pdf_file.pdf", 504, 648);

    @note     Righe e colonne del PDF vengono create con una iterazione.
              L'array pos conserva lo stato della posizione nel documento PDF
              durante il posizionamento delle righe e delle colonne.
              L'aggiunta di una riga non si traduce in alcuna azione effettiva
              sul PDF, ma setta i valori nell'array pos per il successivo
              posizionamento di tutte le colonne.
              La posizione della riga precedente guida il posizionamento
              di quella successiva mediante le variabili prev_row_h e row_h
  */

// Renderizza una pagina del documento PDF
static void mdt_pdf_add_page(apr_pool_t *mp, cairo_t *dst_cr, apr_array_header_t *page_ar, double h) {
  double pos[5] = {0}, bord_y[16] = {0};
  const char *page_entry, *meta, *data, *meta_type;
  apr_array_header_t *page_entry_ar = NULL, *meta_ar;
  // Queste variabili definiscono l'altezza delle righe corrente e precedente
  float prev_row_h = 0, row_h;
  // Inizializzo il contatore dei bordi
  int count_bord_y = 0;
  // Se l'array degli elementi di pagina è valido e non vuoto
  if ((page_ar != NULL) && (page_ar->nelts > 0)) {
    // Eseguo una iterazione
    for (int i = 0; i < page_ar->nelts; i++) {
      // Leggo il prossimo elemento
      page_entry = APR_ARRAY_IDX(page_ar, i, const char*);
      if (page_entry != NULL) {
        // Splitto rispetto al carattere '|'
        page_entry_ar = mdt_split(mp, page_entry, "|");
        if (page_entry_ar != NULL) {
          // L'elemento 0 contiene i metadati
          meta = APR_ARRAY_IDX(page_entry_ar, 0, const char*);
          // L'elemento 1 contiene i dati
          data = APR_ARRAY_IDX(page_entry_ar, 1, const char*);
          // Splitto i metadati rispetto al carattere ';'
          meta_ar = mdt_split(mp, meta, ";");
          // L'elemento 0 dei metadati definisce una riga (r) o una colonna
          meta_type = APR_ARRAY_IDX(meta_ar, 0, const char*);
          if (meta_type[0] == 'r') {
            // L'elemento 1 dei metadati indica l'altezza di riga
            row_h = atof(APR_ARRAY_IDX(meta_ar, 1, const char*));
            // Aggiungo una riga alla pagina calcolandone le dimensioni
            // in base a  quelle della riga corrente (per l'altezza) e della riga
            // precedente (per il posizionamento)
            mdt_pdf_add_row(pos, 0, prev_row_h, row_h);
            // Registro l'altezza di riga per l'utilizzo
            // nella renderizzazione della riga successiva
            prev_row_h = row_h;
            // Incremento il contatore dei bordi
            count_bord_y++;
          }
          else {
            // L'elemento corrente non è una riga, aggiungo una colonna
            mdt_pdf_add_col(mp, dst_cr, pos, meta_ar, data, &bord_y[0], count_bord_y);
          }
        }
      }
    }

    // Azzero il contatore dei bordi
    // e la dimensione della riga precedente
    count_bord_y = 0;
    prev_row_h = 0;

    for (int i = 0; i < page_ar->nelts; i++) {
      // leggo la prossima riga
      page_entry = APR_ARRAY_IDX(page_ar, i, const char*);
      // Splitto rispetto al carattere '|'
      page_entry_ar = mdt_split(mp, page_entry, "|");
      // L'elemento 0 contiene i metadati
      meta = APR_ARRAY_IDX(page_entry_ar, 0, const char*);
      // splitto i metadati rispetto al carattere ';'
      meta_ar = mdt_split(mp, meta, ";");
      // l'elemento 0 dei metadati indica il riferimento a una riga
      meta_type = APR_ARRAY_IDX(meta_ar, 0, const char*);
      if (meta_type[0] == 'r') {
        // l'elemento 1 dei metadati indica l'altezza di riga
        row_h = atof(APR_ARRAY_IDX(meta_ar, 1, const char*));
        // aggiungo una riga
        if (bord_y[count_bord_y])
          row_h = bord_y[count_bord_y];
        mdt_pdf_add_row(pos, 0, prev_row_h, row_h);
        prev_row_h = row_h;
        count_bord_y ++;
      } else {
        // aggiungo una colonna
        mdt_pdf_add_col_border(mp, dst_cr, pos, meta_ar);
      }
    }
    cairo_show_page(dst_cr);
  }
}

// Renderizza il documento PDF
void mdt_pdf(apr_pool_t *mp, const char *src, const char *dst, double w, double h) {
  apr_array_header_t *src_ar, *page_ar;
  cairo_surface_t *cr_surf;
  cairo_t *cr;
  // Splitto il tracciato per riga
  src_ar = mdt_split(mp, src, "\n");
  if ((src_ar != NULL) && (src_ar->nelts > 0)) {
    // Creo una surface PDF
    cr_surf = cairo_pdf_surface_create(dst, w, h);
    cr = cairo_create(cr_surf);
    // Creo l'array della pagine del documento
    page_ar = apr_array_make(mp, 0, sizeof(const char*));
    if (page_ar != NULL) {
      // Eseguo una iterazione sulle righe del tracciato
      for (int i = 0; i < src_ar->nelts; i++) {
        // Leggo la prossima riga
        const char *curr = APR_ARRAY_IDX(src_ar, i, const char*);
        // Se la riga è vuota e l'array degli elementi di pagina è già stato
        // popolato allora questo è l'inizio di una nuova pagina.
        // Se l'array degli elementi di pagina fosse vuoto allora questa è
        // la prima pagina e in questo caso una riga vuota non è accettabile
        if ((curr[0] == MDT_T_EMPTY) && (page_ar->nelts > 0)) {
          // Aggiungo la pagina fin qui prodotta alla surface PDF
          // e reinizializzo l'array delle righe
          mdt_pdf_add_page(mp, cr, page_ar, h);
          page_ar = apr_array_make(mp, 0, sizeof(const char*));
        } else {
          // Aggiungo la riga corrente all'array degli elementi di pagina
          APR_ARRAY_PUSH(page_ar, const char*) = apr_pstrdup(mp, curr);
        }
      }
    }
    cairo_destroy(cr);
    cairo_surface_destroy(cr_surf);
    cairo_debug_reset_static_data();
    FcFini();
  }
}

// EXAMPLE

// #include <stdio.h>
// #include <stdlib.h>
// #include <apr.h>
// #include <apr_pools.h>
// #include <apr_strings.h>
// #include <apr_tables.h>

// #ifndef _ZET_HAS_PDF
// #define _ZET_HAS_PDF
// #endif

// #include "zet.h"

// int main() {
//   int rv, sz;
//   apr_pool_t *mp;
//   void *buffer;
//   char *er;

//   rv = apr_initialize();
//   if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
//   rv = apr_pool_create(&mp, NULL);
//   if (rv != APR_SUCCESS) exit(EXIT_FAILURE);

//   sz = mdt_file_read(mp, "pdf_file.txt", &buffer, 0, &er);

//   if (sz > 0) {
//     mdt_pdf(mp, buffer, "pdf_file.pdf", 504, 648);
//   } else {
//     printf("Error reading file.\n");
//   }

//   apr_pool_destroy(mp);
//   apr_terminate();
//   return 0;
// }

// r;40
// t;440;1111;0.5;solid;000;111;8;000;times;0;0;0|Hola mundo
// i;40;1111;0.5;solid;000;111;8;000;sans ms;0;40;40|Hello.png
// r;40
// t;140;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|hello
// t;240;1111;0.5;solid;000;111;8;000;sans ms;50;0;0|Lorem ipsum dolor sit amet, consur adipiscing elit. Mauris mollis imperdiet nisi, eget pulvinar orci sodales id. Pellentesque in ipsum quis augue suscipit laoreet.
// t;100;1111;0.5;solid;000;111;8;000;sans ms;8;0;0|Lorem ipsum dolor sit amet, consur adipiscing elit. Mauris mollis imperdiet nisi, eget pulvinar orci sodales id. Pellentesque in ipsum quis augue suscipit laoreet.
// r;40
// t;120;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|testo 4
// t;240;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|testo 5
// t;120;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|testo 6
// r;100
// t;480;1111;0.5;solid;000;111;8;000;sans ms;100;0;0|Lorem ipsum dolor sit amet, consur adipiscing elit. Mauris mollis imperdiet nisi, eget pulvinar orci sodales id. Pellentesque in ipsum quis augue suscipit laoreet. Vivamus mollis massa non felis tristique, eget rhoncus augue blandit. Maecenas finibus ligula vitae finibus consequat. Fusce ac tellus id mi lacinia sodales. Morbi neque mi, tristique nec magna id, tincidunt fringilla sem. Vivamus posuere mattis ligula nec aliquet. Quisque in felis nisl. Aliquam tempus tellus eget ligula auctor placerat. Aliquam sodales eget dui non accumsan. Mauris nisi nibh, ultrices ac erat maximus, scelerisque tincidunt lorem. Duis ipsum erat, tempor sed dapibus a, rutrum placerat urna. Ut venenatis ante ac augue gravida, vitae tincidunt mauris ultricies. Sed sit amet molestie ex, vitae malesuada mauris. Donec ac ullamcorper lacus. Aliquam in ipsum id velit laoreet sagittis a sit amet dolor.

// r;40
// t;440;1111;0.5;solid;000;111;8;000;times;0;0;0|Hola mundo
// i;40;1111;0.5;solid;000;111;8;000;sans ms;0;40;40|Hello.png
// r;40
// t;140;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|hello
// t;240;1111;0.5;solid;000;111;8;000;sans ms;50;0;0|Lorem ipsum dolor sit amet, consur adipiscing elit. Mauris mollis imperdiet nisi, eget pulvinar orci sodales id. Pellentesque in ipsum quis augue suscipit laoreet.
// t;100;1111;0.5;solid;000;111;8;000;sans ms;8;0;0|Lorem ipsum dolor sit amet, consur adipiscing elit. Mauris mollis imperdiet nisi, eget pulvinar orci sodales id. Pellentesque in ipsum quis augue suscipit laoreet.
// r;40
// t;120;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|testo 4
// t;240;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|testo 5
// t;120;1111;0.5;solid;000;111;8;000;sans ms;0;0;0|testo 6
// r;100
// t;480;1111;0.5;solid;000;111;8;000;sans ms;100;0;0|Lorem ipsum dolor sit amet, consur adipiscing elit. Mauris mollis imperdiet nisi, eget pulvinar orci sodales id. Pellentesque in ipsum quis augue suscipit laoreet. Vivamus mollis massa non felis tristique, eget rhoncus augue blandit. Maecenas finibus ligula vitae finibus consequat. Fusce ac tellus id mi lacinia sodales. Morbi neque mi, tristique nec magna id, tincidunt fringilla sem. Vivamus posuere mattis ligula nec aliquet. Quisque in felis nisl. Aliquam tempus tellus eget ligula auctor placerat. Aliquam sodales eget dui non accumsan. Mauris nisi nibh, ultrices ac erat maximus, scelerisque tincidunt lorem. Duis ipsum erat, tempor sed dapibus a, rutrum placerat urna. Ut venenatis ante ac augue gravida, vitae tincidunt mauris ultricies. Sed sit amet molestie ex, vitae malesuada mauris. Donec ac ullamcorper lacus. Aliquam in ipsum id velit laoreet sagittis a sit amet dolor.


#endif /* _MDT_PDF */



#ifdef QRCODE
//EXTRA_LDFLAGS=-lqrencode


// #include "httpd.h"
// #include "http_config.h"
// #include "http_protocol.h"
// #include "qrencode.h"
// #include "zet.h"

// module AP_MODULE_DECLARE_DATA mdt_qrcode_module;

// const char* mdt_qr_map(apr_pool_t *m, QRcode *qr) {
//   int width, len;
//   char *bin, *p;
//   unsigned char *data, value;
//   if (qr == NULL) {
//     return NULL;
//   }
//   width = qr->width;
//   data = qr->data;
//   len = width * width;
//   bin = (char*)apr_palloc(m, len + (width-1) + 1);
//   if (bin == NULL) {
//     return NULL;
//   }
//   memset(bin, 0, len + 1);
//   p = bin;
//   for (int i = 0; i < width; i++) {
//     for (int j = 0; j < width; j++) {
//       value = data[i * width + j];
//       *p++ = (value & 1) ? '1' : '0';
//     }
//     if (i < (width-1))
//       *p++ = '-';
//   }
//   return bin;
// }

// static int mdt_qrcode_request_handler(request_rec *r) {
//   QRcode *qr;
//   const char *buf, *bin, q[] = "HELLO";
//   if (strcmp(r->handler, "qr")) return DECLINED;
//   MDT_APACHE_INITIALIZE(r);
//   MDT_APACHE_AUTHORIZE(r, &z_qrcode_module);
//   qr = QRcode_encodeString(q, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
//   bin = mdt_qr_map(r->pool, qr);
//   buf = apr_psprintf(r->pool, "{\"code\":\"%s\",\"width\":%d}", bin, qr->width);
//   MDT_APACHE_RESPONSE_JSON(r, 0, NULL, buf, MDT_T_JSON);
//   return OK;
// }

// static void mdt_qrcode_register_hooks(apr_pool_t *mp) {
//   ap_hook_handler(z_qrcode_request_handler, NULL, NULL, APR_HOOK_LAST);
// }

// static void* mdt_qrcode_serv_config_make(apr_pool_t *m, server_rec *s) {
//   return (void*)apr_table_make(m, 1);
// }

// static const char* mdt_qrcode_param_set(cmd_parms *p, void *c, const char *v) {
//   void *cfg = ap_get_module_config(p->server->module_config, &z_qrcode_module);
//   apr_table_setn((apr_table_t*)cfg, p->cmd->name, v);
//   return NULL;
// }

// static const command_rec mdt_qrcode_directives[] = {
//   AP_INIT_TAKE1("ZAuthType", mdt_qrcode_param_set, NULL, OR_OPTIONS, ""),
//   AP_INIT_TAKE1("ZAuthFile", mdt_qrcode_param_set, NULL, OR_OPTIONS, ""),
//   {NULL}
// };

// module AP_MODULE_DECLARE_DATA mdt_qrcode_module = {
//   STANDARD20_MODULE_STUFF,
//   NULL,
//   NULL,
//   mdt_qrcode_serv_config_make,
//   NULL,
//   mdt_qrcode_directives,
//   mdt_qrcode_register_hooks
// };









// <!DOCTYPE html>
// <html lang="en">
// <head>
//   <meta charset="UTF-8">
//   <meta http-equiv="X-UA-Compatible" content="IE=edge">
//   <meta name="viewport" content="width=device-width, initial-scale=1.0">
//   <title>Document</title>
// </head>
// <body>
//   <canvas id="myCanvas"></canvas>
//   <script>
//     // Definiamo una variabile per la base URL dell'API
//     const endp = '/qr?s=hello';

//     function qrcode(data) {
//       // Otteniamo il riferimento al canvas HTML
//       const canvas = document.getElementById("myCanvas");
//       // Otteniamo il contesto di rendering 2D del canvas
//       const ctx = canvas.getContext("2d");
//       const squareSize = 10;
//       // Impostiamo la dimensione del canvas
//       canvas.width = data.width * squareSize;
//       canvas.height = canvas.width;
//       // Definiamo la stringa di caratteri
//       // Definiamo la dimensione di ogni quadrato
      
//       let code = data.code.split('-');
//       // Iteriamo sulla stringa di caratteri
//       for (let j = 0; j < code.length; j++) {
//         const y = j*squareSize;
//         const str = code[j];
//         for (let i = 0; i < str.length; i++) {
//           // Calcoliamo le coordinate x e y del quadrato corrente
//           const x = i * squareSize;
          
//           // Disegniamo il quadrato corrente
//           if (str.charAt(i) === "1") {
//             ctx.fillStyle = "black";
//           } else {
//             ctx.fillStyle = "white";
//           }
//           ctx.fillRect(x, y, squareSize, squareSize);
//         }
//       }
//     }

//     // Funzione per effettuare una richiesta Ajax
//     function ajaxRequest(method, url, data) {
//       return new Promise((resolve, reject) => {
//         const xhr = new XMLHttpRequest();
//         xhr.open(method, url);
//         xhr.setRequestHeader('Content-Type', 'application/json');
//         xhr.onload = function() {
//           if (xhr.status === 200) {
//             resolve(JSON.parse(xhr.responseText));
//           } else {
//             reject(xhr.statusText);
//           }
//         };
//         xhr.onerror = function() {
//           reject('Errore di connessione');
//         };
//         xhr.send(JSON.stringify(data));
//       });
//     }

//     ajaxRequest('GET', `${endp}`)
//       .then(data => {
//         if (data.err == 0) {
//           qrcode(data.obj);
//         }
//       })
//       .catch(error => console.error(error));
//   </script>
// </body>
// </html>

#endif


#ifdef BM25

#include "service.h"

//! ----------------------------------------------------------------------------
//! Implementazione di OKAPI BM25
//! ----------------------------------------------------------------------------
//!
//!                         f(qi, D) • (k1 + 1)
//! ∑ IDF(qi) • -------------------------------------------
//!             f(qi, D) + k1 • [ 1 - b + b • (|D|/avgdl) ]
//!
//! D        = Documento
//! IDF(qi)  = Peso del termine i-esimo nella collezione di documenti
//! f(qi, D) = Frequenza del termine i-esimo nel documento
//! |D|      = Lunghezza del documento
//! qi       = Termine i-esimo della query
//! b, k1    = Parametri liberi b: 0.75, k1: 1.2
//! avgdl    = Lunghezza media di un documento
//! k1       = Permette di determinare la saturazione della frequenza
//!            di un termine limitandone l'influenza sul il punteggio
//!            di un dato documento
//!
//!          N - n[qi] + 0.5
//! IDF = ln ---------------
//!            n[qi] + 0.5
//!
//! N     = Numero totale dei documenti
//! n[qi] = Numero dei documenti contenenti la query
//!         computato mediante la funzione n_qi()

//! ----------------------------------------------------------------------------

//! \brief      Calcola il numero di documenti nella collezione
//!             contenenti l'i-esimo termine della query di ricerca
//! 
//! \param  D   (array)     Array dei documenti della collezione
//! \param  qi  (string)    I-esimo termine della query di ricerca
//!
int n_qi(apr_array_header_t *docs, const char *qi)
{
    const char *w;
    apr_array_header_t *doc;
    int count = 0;
    for (int i = 0; i < docs->nelts; i ++) {
        doc = APR_ARRAY_IDX(docs, i, apr_array_header_t*);
        for (int j = 0; j < doc->nelts; j++) {
            w = APR_ARRAY_IDX(docs, j, const char*);
            if (strcmp(w, qi)) continue;
            count ++;
            break;
        }
    }
    return count;
}

//! \brief      Inverse Document Frequency
//!             Calcola il peso del termine i-esimo nel documento
//!
//! \param  D   (array)     Array dei documenti 
//! \param  qi  (string)    Termine i-esimo
//!
double IDF_qi(apr_array_header_t *D, const char *qi)
{
    int N = D->nelts;
    int nqi = n_qi(D, qi);
    return log(((((double)N - (double)nqi + 0.5)/((double)nqi + 0.5)) + 1));
}

//! \brief      Frequenza del termine qi nel documento d
//!
//! \param  d   (array)     Documento 
//! \param  qi  (string)    Termine i-esimo
//!
double f_qi_d(apr_array_header_t *d, const char *qi)
{
    int count = 0;
    const char *w;
    for (int i = 0; i < d->nelts; i++) {
        w = APR_ARRAY_IDX(d, i, const char*);
        if (strncasecmp(w, qi, (int)strlen(qi)) == 0) count ++;
    }
    return ((double)count/(double)d->nelts);
}

//! \brief      Lunghezza media di un documento della collezione
//!
//! \param  D   (array)     Collezione dei documenti
//!
double avg_dl(apr_array_header_t *D)
{
    int Dlen;
    apr_array_header_t *d;
    Dlen = 0;
    for (int i = 0; i < D->nelts; i++) {
        d = APR_ARRAY_IDX(D, i, apr_array_header_t*);
        Dlen += d->nelts;
    }
    return ((double)Dlen/(double)D->nelts);
}

//! \brief      Associa ad ogni documento della collezione un punteggio
//!             relativo alla sua pertinenza rispetto alla query
//!
//! \param  mp  (apr_pool_t)  Pool di memoria
//! \param  D   (array)       Collezione dei documenti
//! \param  Q   (array)       Query di ricerca
//! \param  b   (float)       (0.75)
//! \param  k1  (float)       ([1.2 - 2])
//!
apr_array_header_t* bm25(apr_pool_t *mp, apr_array_header_t *D,
                         apr_array_header_t *Q, float b, float k1)
{
    double avgdl, fqid, idfqi, score;
    const char *qi;
    apr_array_header_t *d;
    apr_array_header_t *scores;
    // Calcolo la lunghezza media di un documneto della collezione
    avgdl = avg_dl(D);
    scores = apr_array_make(mp, D->nelts, sizeof(double));
    // Ripeto per ogni documento
    for (int i = 0; i < D->nelts; i++) {
        d = APR_ARRAY_IDX(D, i, apr_array_header_t*);
        score = 0;
       // Ripeto per ogni termine nella query
        for (int j = 0; j < Q->nelts; j++) {
            // Estraggo il prossimo termine
            qi = APR_ARRAY_IDX(Q, j, const char*);
            idfqi = IDF_qi(D, qi); // IDF del termine
            fqid = f_qi_d(d, qi);  // Frequenza del termine
            // Calcolo lo score
            score += idfqi * (
                (fqid * (k1 + 1)) /
                (fqid + k1 * (1 - b + b * ((double)d->nelts / avgdl)))
            );
        }
        APR_ARRAY_PUSH(scores, double) = score;
    }
    return scores;
}

const char *docs[3] = {
  "hello world", "hola mundo", "ciao a tutti"
};

int mt_service_run(mt_t *mt)
{
  apr_status_t rv;
  apr_array_header_t *query, *documents, *doc, *scores, *tmp_ar;
  const char *q;
  void *buffer;
  const char msg[] = "No score!";
  mt_response_content_type_set(mt, "text/plain;charset=utf8");
  dbg_("%s\n", mt->request->query);
  if (mt->request->args != NULL) {
    dbg_("TEST1\n");
    q = apr_table_get(mt->request->args, "q");
    if (q != NULL) {
    dbg_("TEST2\n");
      query = mt_split(mt->pool, q, " ");
      if (query) {
        documents = apr_array_make(mt->pool, 3, sizeof(apr_array_header_t*));
        for (int i = 0; i < 3; i++) {
          doc = mt_split(mt->pool, docs[i], " ");
          if (doc) {
            APR_ARRAY_PUSH(documents, apr_array_header_t*) = doc;
          }
        }
        scores = bm25(mt->pool, documents, query, 0.75, 1.2);
        if (scores != NULL && scores->nelts > 0) {
          tmp_ar = apr_array_make(mt->pool, scores->nelts, sizeof(const char*));
          for (int i = 0; i < documents->nelts; i++) {
            double s = APR_ARRAY_IDX(scores, i, double);
            APR_ARRAY_PUSH(tmp_ar, const char*) = apr_psprintf(mt->pool, "Score: %lf in '%s'", s, docs[i]);
          }
          buffer = (void*)apr_psprintf(mt->pool, "Query: %s,\n%s\n", q, mt_join(mt->pool, tmp_ar, "\n"));
          mt_response_buffer_set(mt, buffer, strlen(buffer));
          return 200;
        }
      }
    }
  }
  mt_response_buffer_set(mt, (void*)msg, strlen(msg));
  return 200;
}

int main(int argc, char **argv)
{
  if (argc < 2) mt_usage(argv[0]);
  mt_config_t *cnf;
  mt_config_init(cnf, ENV_NS);
  mt_daemonize();
  mt_serve(argv[1], cnf, 1000);
  mt_config_free(cnf);
  return 0;
}


#endif



#ifdef schema
// =============================================================================
// schema
// =============================================================================

#ifdef schema


#include "apr.h"
#include "apr_pools.h"
#include "apr_tables.h"
#include "apr_strings.h"
#include "apr_escape.h"
#include "hlp_dbd.h"
#include "hlp_schema.h"

#define HLP_DBD_SCHEMA_MYSQL 0x01
#define HLP_DBD_SCHEMA_PGSQL 0x02
#define HLP_DBD_SCHEMA_SQLITE3 0x03
#define HLP_DBD_SCHEMA_MSSQL 0x04

typedef apr_array_header_t*(*hlp_schema_tab_fn_t) (apr_pool_t *mp, hlp_dbd_t *dbd, const char *tab);
typedef apr_array_header_t*(*hlp_schema_col_fn_t) (apr_pool_t *mp, hlp_dbd_t *dbd, const char *tab, const char *col);
typedef const char*(*hlp_schema_inf_fn_t) (apr_pool_t *mp, hlp_dbd_t *dbd);

typedef struct hlp_schema_t {
  int err;
  const char *log;
  const char *tab;
  int dbd_server_type;
  apr_array_header_t *att;
  hlp_schema_tab_fn_t tb_name_fn;
  hlp_schema_tab_fn_t cl_attr_fn;
  hlp_schema_tab_fn_t pk_attr_fn;
  hlp_schema_tab_fn_t fk_tabs_fn;
  hlp_schema_tab_fn_t fk_attr_fn;
  hlp_schema_tab_fn_t un_attr_fn;
  hlp_schema_col_fn_t cl_name_fn;
  hlp_schema_col_fn_t id_last_fn;
  hlp_schema_inf_fn_t db_vers_fn;
  apr_array_header_t *pk_attrs;
  apr_array_header_t *unsigned_attrs;
  apr_array_header_t *refs_attrs;
} hlp_schema_t;

apr_array_header_t* hlp_mysql_tb_name(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb) {
  const char *pt =
  "SELECT table_name "
  "FROM INFORMATION_SCHEMA.tables WHERE table_name='%s'";
  const char *sql = apr_psprintf(mp, pt, tb);
  return hlp_dbd_select(mp, dbd, sql);
}

apr_array_header_t* hlp_mysql_cl_name(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb, const char *cl) {
  const char *pt =
  "SELECT column_name FROM INFORMATION_SCHEMA.columns "
  "WHERE table_name='%s' AND column_name='%s'";
  const char *sql = apr_psprintf(mp, pt, tb, cl);
  return hlp_dbd_select(mp, dbd, sql);
}

apr_array_header_t* hlp_mysql_cl_attr(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb) {
  const char *pt =
  "SELECT ordinal_position as ordinal_position,"
  "table_name as table_name,"
  "column_name as column_name,"
  "(case when column_default is null then 'null' else column_default end) as column_default, "
  "data_type as data_type,"
  "(case when character_set_name is null then 'null' else character_set_name end) as character_set_name, "
  "column_type as column_type,"
  "(case when column_key is null then 'null' else column_key end) as column_key,"
  "(case when (column_comment is null or COLUMN_COMMENT like '') then 'null' else COLUMN_COMMENT end) as column_comment,"
  "(column_type LIKE '%%unsigned%%') as is_unsigned,"
  "0 as is_primary_key,"
  "0 as is_foreign_key,"
  "(extra LIKE 'auto_increment') as is_auto_increment,"
  "(is_nullable LIKE 'YES') as is_nullable,"
  "(!isnull(numeric_precision)) as is_numeric,"
  "(isnull(numeric_precision)) as is_string,"
  "(data_type LIKE 'date') as is_date,"
  "(column_type LIKE 'tinyint(1) unsigned') as is_boolean,"
  "'null' as column_options,"
  "'null' as referenced_schema,"
  "'null' as referenced_table,"
  "'null' as referenced_column,"
  "0 as is_referenced_pk_multi,"
  "'null' as referenced_pk "
  "FROM INFORMATION_SCHEMA.columns WHERE table_name='%s' "
  "ORDER BY ordinal_position ASC";
  const char *sql = apr_psprintf(mp, pt, tb);
  apr_array_header_t*ret =  hlp_dbd_select(mp, dbd, sql);
  return ret;
}

apr_array_header_t* hlp_mysql_pk_attr(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb) {
  const char *pt =
  "SELECT c.column_name FROM "
  "INFORMATION_SCHEMA.columns AS c JOIN INFORMATION_SCHEMA.statistics AS s "
  "ON s.column_name=c.column_name AND s.table_schema=c.table_schema AND "
  "s.table_name=c.table_name WHERE !isnull(s.index_name) AND "
  "s.index_name LIKE 'PRIMARY' AND c.table_name='%s'";
  const char *sql = apr_psprintf(mp, pt, tb);
  return hlp_dbd_select(mp, dbd, sql);
}

apr_array_header_t* hlp_mysql_un_attr(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb) {
  return NULL;
}

apr_array_header_t* hlp_mysql_fk_tabs(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb) {
  const char *pt = 
  "SELECT table_name FROM INFORMATION_SCHEMA.key_column_usage "
  "WHERE referenced_table_name='%s'";
  const char *sql = apr_psprintf(mp, pt, tb);
  return hlp_dbd_select(mp, dbd, sql);
}

apr_array_header_t* hlp_mysql_fk_attr(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb) {
  const char *pt =
  "SELECT column_name,referenced_table_schema referenced_schema,"
  "referenced_table_name referenced_table,"
  "referenced_column_name referenced_column "
  "FROM INFORMATION_SCHEMA.key_column_usage "
  "WHERE referenced_column_name IS NOT NULL AND table_name='%s'";
  const char *sql = apr_psprintf(mp, pt, tb);
  return hlp_dbd_select(mp, dbd, sql);
}

apr_array_header_t* hlp_mysql_id_last(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tb, const char *pk) {
  const char *sql = apr_pstrdup(mp, "SELECT last_insert_id() as last_id");
  return hlp_dbd_select(mp, dbd, sql);
}

const char* hlp_mysql_version(apr_pool_t *mp, hlp_dbd_t *dbd) {
  apr_array_header_t *res = hlp_dbd_select(mp, dbd, "SELECT version() version");
  if (res != NULL) {
    apr_table_t *t = APR_ARRAY_IDX(res, 0, apr_table_t*);
    if (t != NULL) return apr_table_get(t, "version");
  }
  return NULL;
}

apr_array_header_t* hlp_sqlite3_tb_name(apr_pool_t *mp, hlp_dbd_t *d, const char *tb) {
  const char *sql = apr_psprintf(mp, "PRAGMA table_info(%s)", tb);
  apr_array_header_t *res = hlp_dbd_select(mp, d, sql);
  if (res == NULL) return NULL;
  apr_table_t *tab = APR_ARRAY_IDX(res, 0, apr_table_t*);
  apr_table_set(tab, "table_name", tb);
  return res;
}

apr_array_header_t* hlp_sqlite3_cl_name(apr_pool_t *mp, hlp_dbd_t *d, const char *tb, const char *cl) {
  const char *sql, *col;
  apr_array_header_t *res;
  apr_table_t *tab;
  sql = apr_psprintf(mp, "PRAGMA table_info(%s)", tb);
  if (sql == NULL) return NULL;
  res = hlp_dbd_select(mp, d, sql);
  if (res == NULL || res->nelts <= 0) return NULL;
  for (int i = 0; i < res->nelts; i++) {
    tab = APR_ARRAY_IDX(res, i, apr_table_t*);
    col = apr_table_get(tab, "name");
    if (col == NULL) continue;
    if (strcmp(col, cl) == 0) return res;
  }
  return NULL;
}

apr_array_header_t* hlp_sqlite3_cl_attr(apr_pool_t *mp, hlp_dbd_t *d, const char *tb) {
  const char *pt =
  "SELECT t.cid+1 ordinal_position,'%s' table_name,t.name column_name,"
  "t.dflt_value column_default,t.type data_type,e.encoding character_set_name,"
  "t.type column_type,null column_key,null column_comment,0 is_unsigned,"
  "t.pk is_primary_key,0 is_foreign_key,"
  "CASE WHEN ((SELECT 1 FROM sqlite_master AS m WHERE "
  "m.'name'='%s' AND lower(sql) LIKE '%%autoincrement%%')=1) AND (t.'pk'=1) "
  "THEN '1' ELSE '0' END is_auto_increment,"
  "CASE WHEN t.'notnull'='0' THEN '0' ELSE '1' END is_nullable,"
  "CASE WHEN lower(t.'type')='integer' OR lower(t.'type')='numeric' OR "
  "lower(t.'type')='real' THEN '1' ELSE '0' END is_numeric,"
  "CASE WHEN lower(t.'type')='text' THEN '1' ELSE '0' END is_string,"
  "0 as is_date,0 as is_boolean,null column_options,null referenced_schema,"
  "null referenced_table,null referenced_column,0 is_referenced_pk_multi,"
  "null referenced_pk FROM "
  "pragma_table_info('%s') AS t,pragma_encoding AS e,"
  "sqlite_master AS m WHERE m.name='%s'";
  const char *sql = apr_psprintf(mp, pt, tb, tb, tb, tb);
  if (sql == NULL) return NULL;
  return hlp_dbd_select(mp, d, sql);
}

apr_array_header_t* hlp_sqlite3_pk_attr(apr_pool_t *mp, hlp_dbd_t *d, const char *tb) {
  const char *sql, *attrib; //, *encoding = NULL;
  apr_array_header_t *res, *retv;
  apr_table_t *tab;
  sql = apr_psprintf(mp, "PRAGMA table_info(%s)", tb);
  if (sql == NULL) return NULL;
  res = hlp_dbd_select(mp, d, sql);
  if (res == NULL || res->nelts <= 0) return NULL;
  retv = apr_array_make(mp, 1, sizeof(apr_table_t*));
  if (retv == NULL) return NULL;
  for (int i = 0; i < res->nelts; i++) {
    tab = APR_ARRAY_IDX(res, i, apr_table_t*);
    if ((attrib = apr_table_get(tab, "pk")) == NULL) continue;
    if (atoi(attrib)) {
      if ((attrib = apr_table_get(tab, "name")) == NULL) continue;
      apr_table_set(tab, "column_name", attrib);
      apr_table_unset(tab, "cid");
      apr_table_unset(tab, "name");
      apr_table_unset(tab, "type");
      apr_table_unset(tab, "notnull");
      apr_table_unset(tab, "dflt_value");
      apr_table_unset(tab, "pk");
      APR_ARRAY_PUSH(retv, apr_table_t*) = tab;
    }
  }
  return retv;
}

apr_array_header_t* hlp_sqlite3_un_attr(apr_pool_t *mp, hlp_dbd_t *d, const char *tb) {
  return NULL;
}

apr_array_header_t* hlp_sqlite3_fk_tabs(apr_pool_t *mp, hlp_dbd_t *d, const char *tb) {
  const char *pt =
  "SELECT m.name table_name FROM sqlite_master m "
  "JOIN pragma_foreign_key_list(m.name) p ON m.name!=p.'table' "
  "AND p.'table'='%s' WHERE m.type='table' ORDER BY m.name";
  const char *sql = apr_psprintf(mp, pt, tb);
  return hlp_dbd_select(mp, d, sql);
}

apr_array_header_t* hlp_sqlite3_fk_attr(apr_pool_t *mp, hlp_dbd_t *d, const char *tb) {
  const char *sql, *attrib;
  apr_array_header_t *res;
  apr_table_t *tab;
  sql = apr_psprintf(mp, "PRAGMA foreign_key_list(%s)", tb);
  res = sql != NULL ? hlp_dbd_select(mp, d, sql) : NULL;
  if (res == NULL || res->nelts <= 0) return NULL;
  for (int i = 0; i < res->nelts; i++) {
    tab = APR_ARRAY_IDX(res, i, apr_table_t*);
    if (tab == NULL || (apr_table_elts(tab))->nelts <= 0) continue;
    if((attrib = apr_table_get(tab, "from")) == NULL) continue;
    apr_table_set(tab, "column_name", attrib);
    apr_table_set(tab, "is_foreign_key", "1");
    apr_table_set(tab, "referenced_schema", "null");
    if ((attrib = apr_table_get(tab, "table")) == NULL) continue;
    apr_table_set(tab, "referenced_table", attrib);
    if ((attrib = apr_table_get(tab, "to")) == NULL) continue;
    apr_table_set(tab, "referenced_column", attrib);
    apr_table_unset(tab, "id");
    apr_table_unset(tab, "seq");
    apr_table_unset(tab, "table");
    apr_table_unset(tab, "from");
    apr_table_unset(tab, "to");
    apr_table_unset(tab, "table");
    apr_table_unset(tab, "on_update");
    apr_table_unset(tab, "on_delete");
    apr_table_unset(tab, "match");
  }
  return res;
}

apr_array_header_t* hlp_sqlite3_id_last(apr_pool_t *mp, hlp_dbd_t *d, const char *tb, const char *pk) {
  const char *sql = apr_pstrdup(mp, "SELECT last_insert_rowid()");
  return hlp_dbd_select(mp, d, sql);
}

const char* hlp_sqlite3_version(apr_pool_t *mp, hlp_dbd_t *d) {
  const char *sql = apr_pstrdup(mp, "SELECT sqlite_version() as version");
  apr_array_header_t *res = hlp_dbd_select(mp, d, sql);
  if (res != NULL && res->nelts > 0) {
    apr_table_t *t = APR_ARRAY_IDX(res, 0, apr_table_t*);
    if (t != NULL) return apr_table_get(t, "version");
  }
  return NULL;
}

apr_array_header_t* hlp_schema_attr_get(hlp_schema_t *schema) {
  return schema->att;
}

const char* hlp_schema_table_get(hlp_schema_t *schema) {
  return schema->tab;
}

apr_array_header_t* hlp_schema_get_col_attrs(apr_pool_t *mp, hlp_dbd_t *dbd, hlp_schema_t *schema, const char *tab) {
  apr_array_header_t*ret =schema->cl_attr_fn(mp, dbd, tab);
  return ret;
}

apr_array_header_t* hlp_schema_get_pk_attrs(apr_pool_t *mp, hlp_dbd_t *dbd, hlp_schema_t *schema, const char *tab)
{
  return schema->pk_attr_fn(mp, dbd, tab);
}

apr_array_header_t* hlp_schema_get_unsig_attrs(apr_pool_t *mp, hlp_dbd_t *dbd, hlp_schema_t *schema, const char *tab)
{
  return schema->dbd_server_type ==  HLP_DBD_SCHEMA_MYSQL
    ? NULL 
    : schema->un_attr_fn(mp, dbd, tab);
}

apr_array_header_t* hlp_schema_get_refs_attrs(apr_pool_t *mp, hlp_dbd_t *dbd,
                                                 hlp_schema_t *schema,
                                                 const char *tab)
{
  return schema->fk_attr_fn(mp, dbd, tab);
}

int hlp_schema_update_attrs(apr_pool_t *mp, hlp_dbd_t *dbd,
                               hlp_schema_t *schema)
{
  const char *c_name, *c_pk_name, *c_uns_name, *c_rf_name;
  for (int i = 0; i < schema->att->nelts; i++) {
    c_name = hlp_dbd_field_value(schema->att, i, "column_name");
    if (c_name == NULL) continue;
    /// Updates primary key attributes
    if (schema->pk_attrs != NULL && schema->pk_attrs->nelts > 0) {
      for (int j = 0; j < schema->pk_attrs->nelts; j ++) {
        c_pk_name = hlp_dbd_field_value(schema->pk_attrs, j, "column_name");
        if (c_pk_name == NULL) continue;
        if (strcmp(c_name, c_pk_name) != 0) continue;
        hlp_dbd_field_set(schema->att, i, "is_primary_key", "1");
      }
    }
    /// Updates unsigned attributes
    if (schema->unsigned_attrs != NULL && schema->unsigned_attrs->nelts > 0) {
      for (int j = 0; j < schema->unsigned_attrs->nelts; j ++) {
        c_uns_name = hlp_dbd_field_value(schema->unsigned_attrs, j, "column_name");
        if (c_uns_name == NULL) continue;
        if (strcmp(c_name, c_uns_name) != 0) continue;
        hlp_dbd_field_set(schema->att, i, "is_unsigned", "1");
      }
    }
    /// Updates foreign key attributes
    if (schema->refs_attrs != NULL && schema->refs_attrs->nelts > 0) {
      for (int j = 0; j < schema->refs_attrs->nelts; j ++) {
        c_rf_name = hlp_dbd_field_value(schema->refs_attrs, j, "column_name");
        if (c_rf_name == NULL) continue;
        if (strcmp(c_name, c_rf_name) != 0) continue;
        hlp_dbd_field_set(schema->att, i, "is_foreign_key", "1");
        hlp_dbd_field_set(schema->att, i, "referenced_schema",
                         hlp_dbd_field_value(schema->refs_attrs,
                         j, "referenced_schema"));
        hlp_dbd_field_set(schema->att, i, "referenced_table",
                         hlp_dbd_field_value(schema->refs_attrs,
                         j, "referenced_table"));
        hlp_dbd_field_set(schema->att, i, "referenced_column",
                         hlp_dbd_field_value(schema->refs_attrs,
                         j, "referenced_column"));
        const char *rt = hlp_dbd_field_value(schema->refs_attrs,
                                            j, "referenced_table");
        apr_array_header_t *rk = hlp_schema_get_pk_attrs(mp, dbd, schema, rt);
        if (rk == NULL || rk->nelts <= 0) continue;
        if (rk->nelts <= 1) {
          hlp_dbd_field_set(schema->att, i, "referenced_pk",
                           hlp_dbd_field_value(rk, 0, "column_name"));
          continue;
        }
        apr_array_header_t *rk_names =
          apr_array_make(mp, rk->nelts, sizeof(const char*));
        for (int k = 0; k < rk->nelts; k ++)
          APR_ARRAY_PUSH(rk_names, const char*) =
            hlp_dbd_field_value(rk, k, "column_name");
        hlp_dbd_field_set(schema->att, i, "referenced_pk",
                         apr_array_pstrcat(mp, rk_names, ','));
        hlp_dbd_field_set(schema->att, i, "is_referenced_pk_multi", "1");
      }
    }
  }
  return 0;
}

//hlp_schema_t* hlp_dbd_schema(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tab) {
apr_array_header_t* hlp_dbd_schema(apr_pool_t *mp, hlp_dbd_t *dbd, const char *tab) {
  const char *dbd_driver_name;
  hlp_schema_t *schema = (hlp_schema_t*)apr_palloc(mp, sizeof(hlp_schema_t));
  if (schema == NULL) {
    return NULL;
  }
  schema->err = 0;
  schema->log = NULL;
  schema->dbd_server_type = 0;
  schema->att = NULL;
  schema->tb_name_fn = NULL;
  schema->cl_name_fn = NULL;
  schema->cl_attr_fn = NULL;
  schema->pk_attr_fn = NULL;
  schema->fk_tabs_fn = NULL;
  schema->fk_attr_fn = NULL;
  schema->un_attr_fn = NULL;
  schema->id_last_fn = NULL;
  schema->db_vers_fn = NULL;
  schema->pk_attrs = NULL;
  schema->unsigned_attrs = NULL;
  schema->refs_attrs = NULL;
  schema->tab = apr_pstrdup(mp, tab);
  dbd_driver_name = hlp_dbd_driver_name(dbd);
  if (dbd_driver_name == NULL) {
    return NULL;
  }
  if (strcmp(dbd_driver_name, "mysql") == 0) {
    schema->tb_name_fn = hlp_mysql_tb_name;
    schema->cl_name_fn = hlp_mysql_cl_name;
    schema->cl_attr_fn = hlp_mysql_cl_attr;
    schema->pk_attr_fn = hlp_mysql_pk_attr;
    schema->un_attr_fn = hlp_mysql_un_attr;
    schema->fk_tabs_fn = hlp_mysql_fk_tabs;
    schema->fk_attr_fn = hlp_mysql_fk_attr;
    schema->id_last_fn = hlp_mysql_id_last;
    schema->db_vers_fn = hlp_mysql_version;
  } else if (strcmp(dbd_driver_name, "sqlite3") == 0) {
    schema->tb_name_fn = hlp_sqlite3_tb_name;
    schema->cl_name_fn = hlp_sqlite3_cl_name;
    schema->cl_attr_fn = hlp_sqlite3_cl_attr;
    schema->pk_attr_fn = hlp_sqlite3_pk_attr;
    schema->un_attr_fn = hlp_sqlite3_un_attr;
    schema->fk_tabs_fn = hlp_sqlite3_fk_tabs;
    schema->fk_attr_fn = hlp_sqlite3_fk_attr;
    schema->id_last_fn = hlp_sqlite3_id_last;
    schema->db_vers_fn = hlp_sqlite3_version;
  } else {
    return NULL;
  }
  // #elif defined(WITH_PGSQL)
  // if (strcmp(dbd_driver_name, "pgsql") == 0) {
  //   schema->tb_name_fn = hlp_pgsql_tb_name;
  //   schema->cl_name_fn = hlp_pgsql_cl_name;
  //   schema->cl_attr_fn = hlp_pgsql_cl_attr;
  //   schema->pk_attr_fn = hlp_pgsql_pk_attr;
  //   schema->un_attr_fn = hlp_pgsql_un_attr;
  //   schema->fk_tabs_fn = hlp_pgsql_fk_tabs;
  //   schema->fk_attr_fn = hlp_pgsql_fk_attr;
  //   schema->id_last_fn = hlp_pgsql_id_last;
  //   schema->db_vers_fn = hlp_pgsql_version;
  // } else {
  //   return NULL;
  // }
  // #else
  // return NULL;
  // #endif
  // Estrae gli attributi di colonna
  // per la tabella di riferimento o restituisce un errore
  schema->att = hlp_schema_get_col_attrs(mp, dbd, schema, tab);
  if (schema->att == NULL) {
    return NULL;
  }
  // Estrae gli attributi delle chiavi primarie
  // per la tabella di riferimento o restituisce un errore
  schema->pk_attrs = hlp_schema_get_pk_attrs(mp, dbd, schema, tab);
  if (schema->err) {
    return NULL;
  }
  // Estrae gli attributi delle colonne unsigned
  // per la tabella di riferimento o restituisce un errore
  schema->unsigned_attrs = hlp_schema_get_unsig_attrs(mp, dbd, schema, tab);
  if (schema->err) {
    return NULL;
  }
  // Estrae gli attributi delle chiavi esterne
  // per la tabella di riferimento o restituisce un errore
  schema->refs_attrs = hlp_schema_get_refs_attrs(mp, dbd, schema, tab);
  if (schema->err) {
    return NULL;
  }
  // Sovrascrive gli attributi di colonna
  // con i valori di chiave primaria, unsigned e chiave esterna
  hlp_schema_update_attrs(mp, dbd, schema);
  return schema->att;
}

#endif

#endif