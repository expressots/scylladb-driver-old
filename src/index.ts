import { Cluster } from "@nodepp/example";

const host = "scylla-node"
const scyllaCluster = new Cluster(host);
const result = scyllaCluster.connect();

console.log(result);
