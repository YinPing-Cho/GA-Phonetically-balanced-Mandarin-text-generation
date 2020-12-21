import csv
import numpy as np
import copy
import time
import matplotlib.pyplot as plt
np.random.seed(1)

def mate(parent1, parent2, crossover_points=2):
    points = np.floor(np.random.sample(crossover_points)*parent1.shape[0])
    offspring = np.zeros(parent1.shape[0])
    parent_switch_parent1 = 1

    for i in range(parent1.shape[0]):
        if i in points:
            parent_switch_parent1 = 1 - parent_switch_parent1
        if parent_switch_parent1 == 1:
            offspring[i] = parent1[i]
        else:
            offspring[i] = parent2[i]

    return offspring


def mutate(genotype, mutation_rate):
    for i in range(genotype.shape[0]):
        if np.random.rand() < mutation_rate:
            genotype[i] = np.ceil((np.random.rand()*MAX_tag))
    
    return genotype

def calc_cost(genotype, tag_distro_space, target_distribution):
    pheno_distribution = np.zeros(NUM_phones)
    for i in range(genotype.shape[0]):
        pheno_distribution += tag_distro_space[int(genotype[i])]
    
    cost = np.sum(np.abs(pheno_distribution-target_distribution))
    return cost

def sort_fitness(GA_pool, tag_distro_space, target_distribution):
    all_costs = np.zeros(GA_pool.shape[0])

    for i in range(GA_pool.shape[0]):
        all_costs[i] = calc_cost(GA_pool[i], tag_distro_space, target_distribution)
    
    # sort in ascending order
    individual_indices_sorted = np.argsort(all_costs)
    min_cost = all_costs[individual_indices_sorted[0]]

    return individual_indices_sorted, min_cost


# GA params
GA_params = {
            'population_size': 1000,
            'survival_size': 100,
            'genotype_length': 1000,
            'mutation_rate': 0.2,
            'instance_per_phone_target': 600,
            'zero_padded_sentence_space': 120000,
            'epochs': 1000,
            'parents': 2,
            'time_epochs': 1
            }


reader = csv.reader(open('sentence_phoneDist.csv'))
tag_distro_space = []
for row in reader:
    if row[0] != 'tag':
        tag_distro_space.append(np.asarray([int(x) for x in row[1:]]))

for i in range(GA_params['zero_padded_sentence_space']-len(tag_distro_space)):
    tag_distro_space.append(np.asarray([int(0) for _ in range(tag_distro_space[0].shape[0])]))

tag_distro_space = np.asarray(tag_distro_space)

MAX_tag = len(tag_distro_space)-1
NUM_phones = tag_distro_space[MAX_tag].shape[0]
Target_distribution = np.ones(NUM_phones)*GA_params['instance_per_phone_target']
'''
print('Tag-phone space: ', tag_distro_space.shape)
print('Number of phones: ', NUM_phones)
print('Max tag dict-values:', tag_distro_space[MAX_tag])
print('Max tag:', MAX_tag)
'''
GA_pool = np.ceil(np.random.rand(GA_params['population_size'], GA_params['genotype_length']) * MAX_tag)
print(GA_pool)
print('Max init', np.max(GA_pool))
print('Min init', np.min(GA_pool))
print('Mean init', np.mean(GA_pool))

'''
# test mutation()
print(GA_pool[0])
GA_pool[0] = mutate(GA_pool[0], GA_params['mutation_rate'])
print(GA_pool[0])
old = copy.deepcopy(GA_pool[0])
# test mate()
GA_pool[0] = mate(GA_pool[0], GA_pool[1])
print(GA_pool[0]-old)

# test clac_cost()
for i in range(GA_pool.shape[0]):
    cost = calc_cost(GA_pool[i], tag_distro_space=tag_distro_space, target_distribution=Target_distribution)
    print(cost)
'''
fittest_log = []
for epoch in range(GA_params['epochs']):
    if epoch % GA_params['time_epochs'] == 0:
        start = time.time()

    # rate fitness
    individual_indices_sorted, min_cost = sort_fitness(GA_pool, tag_distro_space, target_distribution=Target_distribution)
    fittest_log.append(min_cost)

    best_genotype = GA_pool[individual_indices_sorted[0]]
    print('min cost', min_cost)

    # mate with survivors
    for i in range(GA_params['survival_size'], GA_params['population_size']):
        parents_indices = np.floor(np.random.sample(GA_params['parents'])*GA_params['survival_size']).astype(int)
        GA_pool[i] = \
        mate(GA_pool[individual_indices_sorted[parents_indices[0]]], GA_pool[individual_indices_sorted[parents_indices[1]]])
    
    # mutate everyone
    for i in range(GA_params['population_size']):
        GA_pool[i] = mutate(GA_pool[i], GA_params['mutation_rate'])

    if epoch % GA_params['time_epochs'] == 0:
        end = time.time()
        print('Epoch: {} Elapsed time for {} epoch(s): {}'.format(epoch, GA_params['time_epochs'], end - start))

out_filename = 'genotype.csv'
with open(out_filename, 'w', newline ='') as myfile:
    wr = csv.writer(myfile, quoting=csv.QUOTE_ALL)
    wr.writerow(best_genotype.tolist())

print('Done with these params: ', GA_params)
print('min cost:', min(fittest_log))
plt.plot(fittest_log)
plt.show()