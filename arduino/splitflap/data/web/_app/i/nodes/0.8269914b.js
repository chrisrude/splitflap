import { S as _, i as c, t as m, d as p } from "../c/index.86993a36.js";
import { d as f, c as i, s as l, u as r, g as u } from "../c/s.bc760b9c.js";
function d(n) { let s; const a = n[1].default, e = i(a, n, n[0], null); return { c() { e && e.c() }, l(t) { e && e.l(t) }, m(t, o) { e && e.m(t, o), s = !0 }, p(t, [o]) { e && e.p && (!s || o & 1) && r(e, a, t, t[0], s ? f(a, t[0], o, null) : u(t[0]), null) }, i(t) { s || (p(e, t), s = !0) }, o(t) { m(e, t), s = !1 }, d(t) { e && e.d(t) } } } function $(n, s, a) { let { $$slots: e = {}, $$scope: t } = s; return n.$$set = o => { "$$scope" in o && a(0, t = o.$$scope) }, [t, e] } class S extends _ { constructor(s) { super(), c(this, s, $, d, l, {}) } } export { S as component };

