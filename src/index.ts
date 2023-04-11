import { Cluster } from "@nodepp/example";

const host = "172.22.0.2"
const scyllaCluster = new Cluster(host);
const result = scyllaCluster.connect();

console.log(result);
